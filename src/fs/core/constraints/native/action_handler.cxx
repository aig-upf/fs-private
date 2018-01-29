
#include <fs/core/constraints/native/action_handler.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/utils/printers/helper.hxx>


namespace fs0 { namespace gecode {
	
std::vector<std::unique_ptr<NativeActionHandler>> NativeActionHandler::create(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_index) {
	std::vector<std::unique_ptr<NativeActionHandler>> managers;
	
	for (unsigned idx = 0; idx < actions.size(); ++idx) {
		// When creating an action CSP handler, it doesn't really make much sense to use the effect conditions.
		auto manager = new NativeActionHandler(*actions[idx], tuple_index);
        managers.emplace_back(manager);
//		if (manager->init(novelty)) {
//			LPT_DEBUG("main", "Generated CSP for action " << *actions[idx] << std::endl <<  *manager << std::endl);
//			managers.push_back(manager);
//		} else {
//			LPT_DEBUG("main", "CSP for action " << *actions[idx] << " is inconsistent ==> the action is not applicable");
//		}
	}
	return managers;
}

// If no set of effects is provided, we'll take all of them into account
NativeActionHandler::NativeActionHandler(const GroundAction& action, const AtomIndex& tuple_index)
	: _tuple_index(tuple_index), _action(action), _precondition_checker(action.getPrecondition(), tuple_index)
{
    const ProblemInfo& info = ProblemInfo::getInstance();

	// Filter out delete effects
	for (const fs::ActionEffect* effect:_action.getEffects()) {
		if (!effect->is_del()) {
            _add_effects.push_back(effect);
        }
	}

    for (const auto* eff:_add_effects) {
        auto lhs_statevar = dynamic_cast<const fs::StateVariable*>(eff->lhs());
        if (!lhs_statevar) throw std::runtime_error(printer() << "Native ActionHandler only available for preconditions with state-variable (fluent-less) heads: " << *eff);

        auto constant_rhs = dynamic_cast<const fs::Constant*>(eff->rhs());
        auto sv_rhs = dynamic_cast<const fs::StateVariable*>(eff->rhs());
        if (!constant_rhs && !sv_rhs) throw std::runtime_error(printer() << "Native ActionHandler accepts only effects with constant or state-variable RHS" << *eff);


        ValueTuple lhs_values;
        for (auto subterm:lhs_statevar->getSubterms()) {
            auto constant = dynamic_cast<const fs::Constant*>(subterm);
            assert(constant);  // Otherwise this could not be a state variable
            lhs_values.push_back(constant->getValue());
        }


        unsigned lhs_symbol = lhs_statevar->getSymbolId();
        if (info.isPredicate(lhs_symbol)) {
            // If the effect is predicative, it must be an add-effect, i.e. have form p(x1,...xn)
            _directly_achievable_tuples.push_back(
                    _tuple_index.to_index(lhs_symbol, lhs_values));

        } else {
            if (constant_rhs) {  // The effect has form X := c
                ValueTuple tuple(lhs_values);
                tuple.push_back(constant_rhs->getValue());
                _directly_achievable_tuples.push_back(
                        _tuple_index.to_index(lhs_symbol, tuple));

            } else {  // The effect has form X := Y
                assert(sv_rhs);
                _directly_achievable_tuples.push_back(INVALID_TUPLE);

                _lhs_rhs_statevars.emplace_back(lhs_statevar->getValue(), sv_rhs->getValue());
            }
        }
    }
}

const fs::Formula* NativeActionHandler::get_precondition() const {
	return _action.getPrecondition();
}

const std::vector<const fs::ActionEffect*>& NativeActionHandler::get_effects() const {
	return _add_effects;
}

void NativeActionHandler::log() const {
	LPT_EDEBUG("heuristic", "Processing action: " << _action);
}

void
NativeActionHandler::process(RPGIndex& graph) {
    // If all effects have form X := c and can only produce tuples that have been already reached
    // in the RPG, we can safely skip it. This is just an optimization to skip the later
    // computations when possible
    bool skip = true;
    for (auto achievable:_directly_achievable_tuples) {
        if (achievable == INVALID_TUPLE || !graph.reached(achievable)) {
            skip = false;
            break;
        }
    }
    if (skip) return;

    // Otherwise, we process the effect to derive the new tuples that it can produce on the current RPG layer
    // We first check whether the precondition holds
    std::vector<AtomIdx> base_support;
    if (!_precondition_checker.check_reachable(graph, base_support)) {
        // ie. the action precondition doesn't hold
        return;
    }


    // If we reached this point, the precondition holds, and the appropriate support is in
    // base_support. We can apply the effects
    unsigned num_novel_atoms = 0;

    for (auto achievable:_directly_achievable_tuples) {
        if (achievable != INVALID_TUPLE && !graph.reached(achievable)) {
            graph.add(achievable, new PlainActionID(&_action), std::vector<AtomIdx>(base_support));
            num_novel_atoms++;
        }
    }

    for (auto vars:_lhs_rhs_statevars) {
        VariableIdx lhsvar = vars.first;
        VariableIdx rhsvar = vars.second;

        const auto& rhs_dom = graph.getRawDomain(rhsvar);
        for (const auto& val:rhs_dom) {
            const auto achievable = _tuple_index.to_index(lhsvar, val);
            if (!graph.reached(achievable)) { // A newly-achievable atom
                std::vector<AtomIdx> eff_support(base_support);
                eff_support.push_back(_tuple_index.to_index(rhsvar, val));
                graph.add(achievable, new PlainActionID(&_action), std::move(eff_support));
                num_novel_atoms++;
            }
        }
    }
    LPT_EDEBUG("heuristic", "The Direct Action Handler produced " << num_novel_atoms << " novel tuples");

}

SimpleFormulaChecker::SimpleFormulaChecker(const fs::Formula* formula,
                                           const AtomIndex& tuple_index)
        : _tuple_index(tuple_index)
{
    const auto* trueval = dynamic_cast<const fs::Tautology*>(formula);
    if (trueval) return; // The check will evaluate always to true

    const auto* prec = dynamic_cast<const fs::Conjunction*>(formula);
    if (!prec) throw std::runtime_error(printer() << "Native NativeActionHandler only available for conjunctive preconditions: " << *formula);

    for (const auto* conjunct:prec->getSubformulae()) {
        const auto* eq_atom = dynamic_cast<const fs::EQAtomicFormula*>(conjunct);
        const auto* neq_atom = dynamic_cast<const fs::NEQAtomicFormula*>(conjunct);
        if (!eq_atom && !neq_atom) throw std::runtime_error(printer() << "Native NativeActionHandler only available for preconditions with simple equality atoms: " << *conjunct);

        const auto* atom = eq_atom ? static_cast<const fs::AtomicFormula*>(eq_atom) :
                           static_cast<const fs::AtomicFormula*>(neq_atom);

        // TODO - CHECK SYMMETRICALLY FOR RHS()!!
        assert(atom->getSubterms().size()==2);
        auto statevar = dynamic_cast<const fs::StateVariable*>(atom->getSubterms()[0]);
        if (!statevar) throw std::runtime_error(printer() << "Native ActionHandler only available for preconditions with simple equality atoms: " << *atom);

        auto value = dynamic_cast<const fs::Constant*>(atom->getSubterms()[1]);
        if (!value) throw std::runtime_error(printer() << "Native ActionHandler only available for preconditions with simple equality atoms: " << *atom);

        if (eq_atom) {
            _equality_atoms.emplace_back(statevar->getValue(), value->getValue());
        } else {
            _inequality_atoms.emplace_back(statevar->getValue(), value->getValue());
        }
    }
}

bool SimpleFormulaChecker::check_reachable(const RPGIndex& graph, SimpleFormulaChecker::SupportT& support) const {
    assert(support.empty());
    SupportT result;

    for (const auto& x:_equality_atoms) {
        auto idx = _tuple_index.to_index(x.first, x.second);
        if (graph.reached(idx)) {
            result.push_back(idx);
        } else { // The action is not applicable
            return false;
        }
    }

    for (const auto& x:_inequality_atoms) {
        const auto& dom = graph.getRawDomain(x.first);
        assert(!dom.empty());
        object_id supporting_value;
        if (dom[0] != x.second) supporting_value = dom[0];
        else if (dom.size() == 1) {
            // i.e. the precondition X \neq c doesn't hold, as there is only one possible value
            // for X in the RPG layer, and it is precisely "c"
            return false;
        } else {
            // There is at least two values for X, ergo the precondition X \neq c must hold
            supporting_value = dom[1];
        }
        result.push_back(_tuple_index.to_index(x.first, supporting_value));
    }

    support = std::move(result);
    return true;
}

} } // namespaces
