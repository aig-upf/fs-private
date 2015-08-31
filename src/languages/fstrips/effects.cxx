
#include <problem_info.hxx>
#include <languages/fstrips/effects.hxx>
#include <problem.hxx>
#include <state.hxx>


namespace fs0 { namespace language { namespace fstrips {

	
std::vector<Term::cptr> ActionEffect::flatten() const {
	std::vector<Term::cptr> res = _lhs->flatten();
	auto rhsf = _rhs->flatten();
	res.insert(res.end(), rhsf.cbegin(), rhsf.cend());
	return res;
}
	
bool ActionEffect::isWellFormed() const {
	auto lhs_var = dynamic_cast<StateVariable::cptr>(_lhs);
	auto lhs_fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(_lhs);
	return lhs_var || lhs_fluent; // The LHS of the effect must be either a state variable or a fluent function.
}

Atom ActionEffect::apply(const State& state) const {
	return Atom(_lhs->interpretVariable(state), _rhs->interpret(state));
}


std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_lhs << " := " << *_rhs;
	return os;
}


} } } // namespaces
