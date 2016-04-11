
#include <problem_info.hxx>
#include <languages/fstrips/effects.hxx>
#include <problem.hxx>
#include <state.hxx>


namespace fs0 { namespace language { namespace fstrips {

ActionEffect::ActionEffect(Term::cptr lhs_, Term::cptr rhs_)
	: _lhs(lhs_), _rhs(rhs_) {
	if (!isWellFormed()) throw std::runtime_error("Ill-formed effect");
}

ActionEffect::~ActionEffect() {
	delete _lhs;
	delete _rhs;
}

ActionEffect::ActionEffect(const ActionEffect& other) :
	_lhs(other._lhs->clone()), _rhs(other._rhs->clone())
{}

std::vector<Term::cptr> ActionEffect::all_terms() const {
	std::vector<Term::cptr> res = _lhs->all_terms();
	auto rhsf = _rhs->all_terms();
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


std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, Problem::getInfo()); }

std::ostream& ActionEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_lhs << " := " << *_rhs;
	return os;
}

ActionEffect::cptr ActionEffect::bind(const Binding& binding, const ProblemInfo& info) const {
	return new ActionEffect(_lhs->bind(binding, info), _rhs->bind(binding, info));
}


} } } // namespaces
