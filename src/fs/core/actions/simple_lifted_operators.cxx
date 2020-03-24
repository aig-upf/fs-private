
#include "simple_lifted_operators.hxx"

#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/utils/binding.hxx>


namespace fs0 {

class CompilationError : public std::runtime_error {
public:
    explicit CompilationError(const std::string& msg) : std::runtime_error(msg) {}
};

SimpleLiftedOperator::simple_term compile_simple_term(const fs::Term* term) {
    auto bv = dynamic_cast<const fs::BoundVariable*>(term);
    auto c = dynamic_cast<const fs::Constant*>(term);

    if (bv) return {SimpleLiftedOperator::term_t::var, bv->getVariableId()};
    if (c) return {SimpleLiftedOperator::term_t::constant, c->getValue()};
    throw CompilationError(fs0::printer() << "Cannot compile atom argument into simple_term: " << *term);
}

std::vector<SimpleLiftedOperator::simple_term> compile_arguments(const std::vector<const fs::Term*>& arguments) {
    std::vector<SimpleLiftedOperator::simple_term> compiled;
    for (const auto term:arguments) {
        compiled.emplace_back(compile_simple_term(term));
    }
    return compiled;
}

SimpleLiftedOperator::atom_t unpack_atom_into_literal(const fs::Term* lhs, const fs::Term* rhs, bool negated, bool allow_statics) {
    SimpleLiftedOperator::simple_term val = compile_simple_term(rhs);

    // Let's deal with each possible case for an atom LHS
    auto sv = dynamic_cast<const fs::StateVariable*>(lhs);
    auto fh = dynamic_cast<const fs::FluentHeadedNestedTerm*>(lhs);
    auto sh = dynamic_cast<const fs::StaticHeadedNestedTerm*>(lhs);

    const fs::NestedTerm* origin = nullptr;
    if (sv) origin = sv->getOrigin();
    else if (fh) origin = fh;
    else if (allow_statics && sh) origin = sh;
    else throw CompilationError(fs0::printer() << "Cannot compile atom left-hand side into SimpleLiftedOperator element: " << *lhs);

    return SimpleLiftedOperator::atom_t(origin->getSymbolId(), compile_arguments(origin->getSubterms()), val, negated);
}

SimpleLiftedOperator::simple_term_equality unpack_atom_into_simple_term_eq(const fs::Term* lhs, const fs::Term* rhs, bool negated) {
    SimpleLiftedOperator::simple_term_equality_t type = negated ?
            SimpleLiftedOperator::simple_term_equality_t::neq : SimpleLiftedOperator::simple_term_equality_t::eq;

    return SimpleLiftedOperator::simple_term_equality(type, compile_simple_term(lhs), compile_simple_term(rhs));
}

void compile_conjunct(const fs::Formula* sub, SimpleLiftedOperator::condition_t& condition) {
    const auto* taut = dynamic_cast<const fs::Tautology*>(sub);
    if (taut) return; // No need to add any conjunct

    const auto* eqf = dynamic_cast<const fs::EQAtomicFormula*>(sub);
    const auto* neqf = dynamic_cast<const fs::NEQAtomicFormula*>(sub);
    const auto* base = dynamic_cast<const fs::RelationalFormula*>(sub);
    if (!eqf && !neqf) throw CompilationError(fs0::printer() << "Cannot compile conjunct into SimpleLiftedOperator: " << *sub);

    bool negated = (bool) neqf;

    // First try to compile into a simple_term_equality:
    try {
        condition.simpleeqs.emplace_back(unpack_atom_into_simple_term_eq(base->lhs(), base->rhs(), negated));
    } catch(CompilationError& e) {
        // If not possible, then try to compile into a standard atom
        condition.fluents.emplace_back(unpack_atom_into_literal(base->lhs(), base->rhs(), negated, true));
    }
}

SimpleLiftedOperator::condition_t compile_condition(const fs::Formula* formula) {
    SimpleLiftedOperator::condition_t condition;

    const auto* conjunction = dynamic_cast<const fs::Conjunction*>(formula);
    if (conjunction) { // Wrap out the conjuncts
        for (const auto& sub:conjunction->getSubformulae()) {
            compile_conjunct(sub, condition);
        }
    } else {
        compile_conjunct(formula, condition);
    }
    return condition;
}


SimpleLiftedOperator compile_schema_to_simple_lifted_operator(const PartiallyGroundedAction& action) {

    // Compile effects
    std::vector<SimpleLiftedOperator::effect_t> effects;
    for (const auto& eff:action.getEffects()) {
        effects.emplace_back(
                compile_condition(eff->condition()),
                unpack_atom_into_literal(eff->lhs(), eff->rhs(), false, false));
    }

    try {
        return SimpleLiftedOperator(compile_condition(action.getPrecondition()), effects);
    } catch(CompilationError& e) {
        throw CompilationError(fs0::printer() << "Cannot compile action into SimpleLiftedOperator. "
                                                 "Reason: " << e.what() << ". Action: " << action);
    }
}


object_id bind_simple_term(
        const SimpleLiftedOperator::simple_term& term,
        const Binding& binding,
        const ProblemInfo& info
) {
    if (term.type == SimpleLiftedOperator::term_t::constant) { // We have an object
        return term.val.o;
    }
    // else we have a variable
    assert(term.type == SimpleLiftedOperator::term_t::var);
    auto varid = term.val.varidx;
    return binding[varid];
}

std::vector<object_id> bind_arguments(
        const std::vector<SimpleLiftedOperator::simple_term>& arguments,
        const Binding& binding,
        const ProblemInfo& info
) {
    std::vector<object_id> interpreted;
    if (arguments.empty()) return interpreted;

    std::size_t sz = arguments.size();
    interpreted.reserve(sz);
    for (const auto& arg:arguments) {
        interpreted.emplace_back(bind_simple_term(arg, binding, info));
    }
    return interpreted;
}

VariableIdx bind_variable(
        uint16_t predicate_id,
        const std::vector<SimpleLiftedOperator::simple_term>& arguments,
        const Binding& binding,
        const ProblemInfo& info
) {
    return info.resolveStateVariable(predicate_id, bind_arguments(arguments, binding, info));
}

// Return the value of the given atom under the given binding and state
object_id evaluate_atom(
        const State& state,
        uint16_t predicate_id,
        const std::vector<SimpleLiftedOperator::simple_term>& arguments,
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
    // First check simple-term (in-)equalities
    for (const auto& atom:condition.simpleeqs) {
        auto lhs = bind_simple_term(atom.lhs, binding, info);
        auto rhs = bind_simple_term(atom.rhs, binding, info);
        if ((atom.is_eq() && lhs != rhs) || (!atom.is_eq() && lhs == rhs)) {
            return false; // early-terminate the evaluation of the conjunction
        }
    }

    // Now check other terms involving non-builtin symbols
    for (const auto& atom:condition.fluents) {
        auto lhs_val = evaluate_atom(state, atom.predicate_id, atom.arguments, binding, info);
        auto val = bind_simple_term(atom.value, binding, info);
        if ((!atom.negated && lhs_val != val) || (atom.negated && lhs_val == val)) {
            return false; // early-terminate the evaluation of the conjunction
        }
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
            assert(!atom.negated); // effect "atoms" cannot be negated, as they are in reality simple assignments
            VariableIdx var = bind_variable(atom.predicate_id, atom.arguments, binding, info);
            atoms.emplace_back(var, bind_simple_term(atom.value, binding, info));
        }
    }
}


} // namespaces