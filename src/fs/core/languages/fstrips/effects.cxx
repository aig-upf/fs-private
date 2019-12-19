
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>


namespace fs0::language::fstrips {

ActionEffect::ActionEffect(const Term* lhs, const Term* rhs, const Formula* condition)
	: _lhs(lhs), _rhs(rhs), _condition(condition) {
	if (!is_acceptable_lhs(lhs)) throw std::runtime_error("Ill-formed effect");
}

ActionEffect::~ActionEffect() {
	delete _lhs;
	delete _rhs;
	delete _condition;
}

ActionEffect::ActionEffect(const ActionEffect& other) :
	_lhs(other._lhs->clone()), _rhs(other._rhs->clone()), _condition(other._condition->clone())
{}

std::vector<const Term*> ActionEffect::all_terms() const {
	std::vector<const Term*> res = fs::all_terms(*_lhs);
	auto rhsf = fs::all_terms(*_rhs);
	res.insert(res.end(), rhsf.cbegin(), rhsf.cend());
	return res;
}

bool ActionEffect::is_acceptable_lhs(const Term* lhs) {
	auto lhs_var = dynamic_cast<const StateVariable*>(lhs);
	auto lhs_fluent = dynamic_cast<const FluentHeadedNestedTerm*>(lhs);
	return lhs_var || lhs_fluent; // The LHS of the effect must be either a state variable or a fluent function.
}

Atom ActionEffect::apply(const State& state) const {
	return Atom(fs::interpret_variable(*_lhs, state), _rhs->interpret(state));
}

bool ActionEffect::applicable(const State& state) const {
	return _condition->interpret(state);
}

std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

std::ostream& ActionEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	if (_condition && !_condition->is_tautology()) {
		os << *_condition << " --> ";
	}
	os << *_lhs << " := " << *_rhs;
	return os;
}

const ActionEffect* ActionEffect::bind(const Binding& binding, const ProblemInfo& info) const {
	const fs::Formula* condition = fs0::language::fstrips::bind(*_condition, binding, info);
	if (condition->is_contradiction()) {
		delete condition;
		return nullptr;
	}

	const Term* bound_lhs = nullptr;
	try {
		// TODO - This is a big ___TEMPORARY___ HACK :-). Which btw might be leaking memory. See issue #18
		bound_lhs = fs::bind(*_lhs, binding, info);
	} catch (const std::out_of_range& e) {
		return nullptr;
	}
	auto bound_rhs = fs::bind(*_rhs, binding, info);
	// As of now, the rationale is: if the effect LHS provokes an exception, it must involve
	// a static state variable, and thus we can ignore it and prune it. It is clearly a flawed
	// reasoning if the expression is complex and involves nested terms, etc., but works for now.

	if (ActionEffect::is_acceptable_lhs(bound_lhs)) {
        return new ActionEffect(bound_lhs, bound_rhs, condition);
	} else {  // The effect is attempting to assign a truth value to a static atom
        const auto* c = dynamic_cast<const Constant*>(bound_lhs);
	    assert(c);
	    // We can only assume that the effect will try to assign the same value to the constant than the one it has
	    // initially, otherwise it would have been flagged as fluent
	    return nullptr;
	}

}

// TODO - Refactor this into a hierarchy of effects, a delete effect should be an object of a particular type, or at least effect should have a method is_delete()
bool ActionEffect::is_predicative() const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return (dynamic_cast<const fs::StateVariable*>(lhs()) && info.isPredicate(dynamic_cast<const fs::StateVariable*>(lhs())->getSymbolId())) ||
		   (dynamic_cast<const fs::FluentHeadedNestedTerm*>(lhs()) && info.isPredicate(dynamic_cast<const fs::FluentHeadedNestedTerm*>(lhs())->getSymbolId()));
}

// TODO - Refactor this into a hierarchy of effects, a delete effect should be an object of a particular type, or at least effect should have a method is_delete()
bool ActionEffect::is_del() const {
	if (!is_predicative()) return false;
	assert(dynamic_cast<const fs::Constant*>(rhs())); // Predicative effects are necessarily bound to a constant (true/false) RHS
	return int(dynamic_cast<const fs::Constant*>(rhs())->getValue()) == 0;
}

bool ActionEffect::is_add() const {
	return is_predicative() && !is_del();
}

ProceduralEffect::ProceduralEffect() {

}

ProceduralEffect::~ProceduralEffect() {
}

ProceduralEffect::ProceduralEffect(const ProceduralEffect& other)
{}

std::ostream& ProceduralEffect::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

std::ostream& ProceduralEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return os << "unnamed_procedural_effect";
}

} // namespaces
