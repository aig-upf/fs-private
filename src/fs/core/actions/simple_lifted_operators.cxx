
#include "simple_lifted_operators.hxx"

#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/printers/helper.hxx>


namespace fs0 {

class CompilationError : public std::runtime_error {
public:
    explicit CompilationError(const std::string& msg) : std::runtime_error(msg) {}
};

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
            throw CompilationError(fs0::printer() << "Cannot compile atom argument into SimpleLiftedOperator: " << *term);
        }
    }
    return compiled;
}

SimpleLiftedOperator::atom_t unpack_atom_into_literal(const fs::Term* lhs, const fs::Term* rhs, bool allow_statics=true) {
    auto c = dynamic_cast<const fs::Constant*>(rhs);
    if (!c) throw CompilationError(fs0::printer() << "Cannot compile effect right-hand side into SimpleLiftedOperator element: " << *rhs);

    // The LHF of an atom must be either a StateVariable or a FluentHeadedNestedTerm; in both cases, we'll
    // deal with the (underlying) FluentHeadedNestedTerm
    auto sv = dynamic_cast<const fs::StateVariable*>(lhs);
    auto fh = dynamic_cast<const fs::FluentHeadedNestedTerm*>(lhs);
    auto sh = dynamic_cast<const fs::StaticHeadedNestedTerm*>(lhs);

    const fs::NestedTerm* origin = nullptr;
    if (sv) origin = sv->getOrigin();
    else if (fh) origin = fh;
    else if (allow_statics && sh) origin = sh;
    else throw CompilationError(fs0::printer() << "Cannot compile effect left-hand side into SimpleLiftedOperator element: " << *lhs);

    return SimpleLiftedOperator::atom_t(origin->getSymbolId(), compile_arguments(origin->getSubterms()), c->getValue());
}

SimpleLiftedOperator::condition_t compile_condition(const fs::Formula* formula) {
    SimpleLiftedOperator::condition_t condition;

    const auto* conjunction = dynamic_cast<const fs::Conjunction*>(formula);
    const auto* taut = dynamic_cast<const fs::Tautology*>(formula);
    const auto* atom = dynamic_cast<const fs::EQAtomicFormula*>(formula);

    if (conjunction) { // Wrap out the conjuncts
        for (const auto& sub:conjunction->getSubformulae()) {
            const auto* sub_atom = dynamic_cast<const fs::EQAtomicFormula*>(sub);
            if (!sub_atom) throw CompilationError(fs0::printer() << "Cannot compile conjunct into SimpleLiftedOperator: " << *sub);
            condition.emplace_back(unpack_atom_into_literal(sub_atom->lhs(), sub_atom->rhs()));
        }

    } else if (atom) {
        condition.emplace_back(unpack_atom_into_literal(atom->lhs(), atom->rhs()));

    } else if (taut) {
        // No need to do anything - the empty vector will be the right precondition
    } else {
        throw CompilationError(fs0::printer() << "Cannot compile condition into SimpleLiftedOperator: " << *formula);
    }

    return condition;
}


SimpleLiftedOperator compile_schema_to_simple_lifted_operator(const PartiallyGroundedAction& action) {

    // Compile effects
    std::vector<SimpleLiftedOperator::effect_t> effects;
    for (const auto& eff:action.getEffects()) {
        effects.emplace_back(
                compile_condition(eff->condition()),
                unpack_atom_into_literal(eff->lhs(), eff->rhs(), false));
    }

    try {
        return SimpleLiftedOperator(compile_condition(action.getPrecondition()), effects);
    } catch(CompilationError& e) {
        throw CompilationError(fs0::printer() << "Cannot compile action into SimpleLiftedOperator. "
                                                 "Reason: " << e.what() << ". Action: " << action);
    }
}

std::vector<object_id> bind_arguments(
        const std::vector<SimpleLiftedOperator::argument_t>& arguments,
        const Binding& binding,
        const ProblemInfo& info
) {
    std::vector<object_id> interpreted;
    if (arguments.empty()) return interpreted;

    std::size_t sz = arguments.size();
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
    return interpreted;
}

VariableIdx bind_variable(
        uint16_t predicate_id,
        const std::vector<SimpleLiftedOperator::argument_t>& arguments,
        const Binding& binding,
        const ProblemInfo& info
) {
    return info.resolveStateVariable(predicate_id, bind_arguments(arguments, binding, info));
}

// Return the value of the given atom under the given binding and state
object_id bind_atom(
        const State& state,
        uint16_t predicate_id,
        const std::vector<SimpleLiftedOperator::argument_t>& arguments,
        const Binding& binding,
        const ProblemInfo& info
        ) {
    const auto& fidx = info.get_fluent_index();
    const auto args = bind_arguments(arguments, binding, info);
    auto it = fidx.find(std::make_pair(predicate_id, args));
    if (it != fidx.end()) { // it's a fluent! Let's just take its value from the state
        return state.getValue(it->second);
    }

    // else it's a static atom, let's take its value from the static symbol-data index
    const auto& function = info.getSymbolData(predicate_id);
    return function.getFunction()(args);
}

bool evaluate_simple_condition(
        const State& state,
        const SimpleLiftedOperator::condition_t& condition,
        const Binding& binding,
        const ProblemInfo& info) {
    for (const auto& conjunct:condition) {
        auto lhs_val = bind_atom(state, conjunct.predicate_id, conjunct.arguments, binding, info);
        if (lhs_val != conjunct.value) return false; // early-terminate the evaluation of the conjunction
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

//    assert(evaluate_simple_condition(state, op.precondition, binding, info));  // A bit expensive, but might be worth checking!
    if (check_precondition) {
        if (!evaluate_simple_condition(state, op.precondition, binding, info)) return;
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