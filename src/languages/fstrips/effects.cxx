
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


std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

std::ostream& ActionEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_lhs << " := " << *_rhs;
	return os;
}

ActionEffect::cptr ActionEffect::bind(const Binding& binding, const ProblemInfo& info) const {
	return new ActionEffect(_lhs->bind(binding, info), _rhs->bind(binding, info));
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
	return dynamic_cast<const fs::Constant*>(rhs())->getValue() == 0;
}

bool ActionEffect::is_add() const {
	return is_predicative() && !is_del();
}



} } } // namespaces
