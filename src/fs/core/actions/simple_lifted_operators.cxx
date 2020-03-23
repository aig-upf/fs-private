
#include "simple_lifted_operators.hxx"

#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <fs/core/actions/actions.hxx>


namespace fs0 {

std::vector<SimpleLiftedOperator::argument_t> compile_arguments(const std::vector<const fs::Term*>& arguments) {
    std::vector<SimpleLiftedOperator::argument_t> compiled;
    for (const auto term:arguments) {
        auto bv = dynamic_cast<const fs::BoundVariable*>(term);
        auto c = dynamic_cast<const fs::Constant*>(term);

        if (bv) {
            compiled.emplace_back(SimpleLiftedOperator::argtype_t::var, bv->getVariableId());
        } else if (c) {
            compiled.emplace_back(SimpleLiftedOperator::argtype_t::constant, c->getValue());
        } else {
            throw std::runtime_error("Cannot compile given action schema into SimpleLiftedOperator");
        }
    }
    return compiled;
}

SimpleLiftedOperator::atom_t unpack_atom_into_lifted_op(const fs::Term* lhs, const fs::Term* rhs) {
    auto c = dynamic_cast<const fs::Constant*>(rhs);
    if (!c) throw std::runtime_error("Cannot compile given action schema into SimpleLiftedOperator");

    // The LHF of an atom must be either a StateVariable or a FluentHeadedNestedTerm; in both cases, we'll
    // deal with the (underlying) FluentHeadedNestedTerm
    auto sv = dynamic_cast<const fs::StateVariable*>(lhs);
    auto fh = dynamic_cast<const fs::FluentHeadedNestedTerm*>(lhs);
    if (sv) fh = sv->getOrigin();

    if (!fh) throw std::runtime_error("Cannot compile given action schema into SimpleLiftedOperator");
    return SimpleLiftedOperator::atom_t(fh->getSymbolId(), compile_arguments(fh->getSubterms()), c->getValue());
}

SimpleLiftedOperator::condition_t compile_condition(const fs::Formula* formula) {
    SimpleLiftedOperator::condition_t condition;

    const auto* conjunction = dynamic_cast<const fs::Conjunction*>(formula);
    const auto* taut = dynamic_cast<const fs::Tautology*>(formula);
    const auto* atom = dynamic_cast<const fs::EQAtomicFormula*>(formula);

    if (conjunction) { // Wrap out the conjuncts
        for (const auto& sub:conjunction->getSubformulae()) {
            const auto* sub_atom = dynamic_cast<const fs::EQAtomicFormula*>(sub);
            if (!sub_atom) throw std::runtime_error("Cannot compile given action schema into SimpleLiftedOperator");
            condition.emplace_back(unpack_atom_into_lifted_op(sub_atom->lhs(), sub_atom->rhs()));
        }

    } else if (atom) {
        condition.emplace_back(unpack_atom_into_lifted_op(atom->lhs(), atom->rhs()));

    } else if (taut) {
        // No need to do anything - the empty vector will be the right precondition
    } else {
        throw std::runtime_error("Cannot compile given ground action into SimpleLiftedOperator");
    }

    return condition;
}


SimpleLiftedOperator compile_schema_to_simple_lifted_operator(const PartiallyGroundedAction& action) {

    // Compile effects
    std::vector<SimpleLiftedOperator::effect_t> effects;
    for (const auto& eff:action.getEffects()) {
        effects.emplace_back(
                compile_condition(eff->condition()),
                unpack_atom_into_lifted_op(eff->lhs(), eff->rhs()));
    }

    return SimpleLiftedOperator(compile_condition(action.getPrecondition()), effects);
}

VariableIdx bind_variable(
        uint16_t predicate_id,
        const std::vector<SimpleLiftedOperator::argument_t>& arguments,
        const Binding& binding,
        const ProblemInfo& info
        ) {

    std::size_t sz = arguments.size();
    std::vector<object_id> interpreted;
    interpreted.reserve(sz);
    for (const auto& arg:arguments) {
        if (arg.type == SimpleLiftedOperator::argtype_t::constant) { // We have an object
            interpreted.emplace_back(arg.val.o);
        } else { // We have a variable
            assert(arg.type == SimpleLiftedOperator::argtype_t::var);
            auto varid = arg.val.varidx;
            assert(binding.binds(varid));
            interpreted.emplace_back(binding[varid]);
        }
    }
    return info.resolveStateVariable(predicate_id, std::move(interpreted));
}

bool evaluate_simple_condition(
        const State& state,
        const SimpleLiftedOperator::condition_t& condition,
        const Binding& binding,
        const ProblemInfo& info) {

    for (const auto& conjunct:condition) {
        VariableIdx var = bind_variable(conjunct.predicate_id, conjunct.arguments, binding, info);
        if (state.getValue(var) != conjunct.value) return false; // early-terminate the evaluation of the conjunction
        assert(info.checkValueIsValid(var, conjunct.value));
    }

    // Note that this also implies that a conjunction with 0 conjuncts is equivalent to true, which is what we want
    return true;
}

void evaluate_simple_lifted_operator(
        const State& state,
        const SimpleLiftedOperator& op,
        const Binding& binding,
        const ProblemInfo& info,
        bool check_precondition,
        std::vector<Atom>& atoms) {

    if (check_precondition) {
        throw std::runtime_error("TO BE IMPLEMENTED");
        // TODO if not applicable, simply return without adding anything to atoms.
    }

    atoms.clear();

    for (const auto& eff:op.effects) {
        if (evaluate_simple_condition(state, eff.condition, binding, info)) {
            const auto& atom = eff.atom;
            VariableIdx var = bind_variable(atom.predicate_id, atom.arguments, binding, info);
            atoms.emplace_back(var, atom.value);
        }
    }
}


} // namespaces