
#include <problem_info.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

namespace fs0 { namespace language { namespace fstrips {

// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef AtomicFormula::Symbol AFSymbol;
std::vector<std::pair<AFSymbol, std::string>> symbol_to_string_init{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
const std::map<AFSymbol, std::string> AtomicFormula::symbol_to_string(symbol_to_string_init.begin(), symbol_to_string_init.end());
const std::map<std::string, AFSymbol> AtomicFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


//! A helper to interpret a vector of terms
template <typename T>
ObjectIdxVector interpret_subterms(const std::vector<Term::cptr>& subterms, const T& assignment) {
	ObjectIdxVector interpreted;
	for (Term::cptr subterm:subterms) {
		interpreted.push_back(subterm->interpret(assignment));
	}
	return interpreted;
}

VariableIdxVector Term::computeScope() const {
	std::set<VariableIdx> set;
	computeScope(set);
	return VariableIdxVector(set.cbegin(), set.cend());
}

std::ostream& Term::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& Term::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << "<unnamed term>";
	return os;
}

void NestedTerm::computeScope(std::set<VariableIdx>& scope) const {
	for (Term::cptr term:_subterms) {
		term->computeScope(scope);
	}
}

std::ostream& NestedTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return printFunction(os, info, _symbol_id, _subterms);
}


StaticHeadedNestedTerm::StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
	: NestedTerm(symbol_id, subterms),
	_function(Problem::getCurrentProblem()->getProblemInfo().getFunctionData(_symbol_id).getFunction())
{}
		 
ObjectIdx StaticHeadedNestedTerm::interpret(const PartialAssignment& assignment) const {
	return _function(interpret_subterms(_subterms, assignment));
}

ObjectIdx StaticHeadedNestedTerm::interpret(const State& state) const {
	return _function(interpret_subterms(_subterms, state));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const PartialAssignment& assignment) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	VariableIdx variable = info.resolveStateVariable(_symbol_id, interpret_subterms(_subterms, assignment));
	return assignment.at(variable);
}

ObjectIdx FluentHeadedNestedTerm::interpret(const State& state) const {
	return state.getValue(interpretVariable(state));
}

VariableIdx FluentHeadedNestedTerm::interpretVariable(const State& state) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	VariableIdx variable = info.resolveStateVariable(_symbol_id, interpret_subterms(_subterms, state));
	return variable;
}

void FluentHeadedNestedTerm::computeSubtermScope(std::set<VariableIdx>& scope) const  {
	return NestedTerm::computeScope(scope);
}

void StateVariable::computeScope(std::set<VariableIdx>& scope) const {
	scope.insert(_variable_id);
}

ObjectIdx StateVariable::interpret(const State& state) const { 
	return state.getValue(_variable_id);
}

std::ostream& StateVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.getVariableName(_variable_id);
	return os;
}

std::ostream& Constant::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << _value;
	return os;
}


AtomicFormula::cptr AtomicFormula::create(AtomicFormula::Symbol symbol, Term::cptr lhs, Term::cptr rhs) {
	if (symbol == AtomicFormula::Symbol::EQ)  return new EQAtomicFormula(lhs, rhs);
	if (symbol == AtomicFormula::Symbol::NEQ) return new NEQAtomicFormula(lhs, rhs);
	if (symbol == AtomicFormula::Symbol::LT)  return new LTAtomicFormula(lhs, rhs);
	if (symbol == AtomicFormula::Symbol::LEQ) return new LEQAtomicFormula(lhs, rhs);
	if (symbol == AtomicFormula::Symbol::GT)  return new GTAtomicFormula(lhs, rhs);
	if (symbol == AtomicFormula::Symbol::GEQ) return new GEQAtomicFormula(lhs, rhs);
	assert(0);
}


void AtomicFormula::computeScope(std::set<VariableIdx>& scope) const {
	lhs->computeScope(scope);
	rhs->computeScope(scope);
}

VariableIdxVector AtomicFormula::computeScope() const {
	std::set<VariableIdx> set;
	computeScope(set);
	return VariableIdxVector(set.cbegin(), set.cend());
}


std::ostream& AtomicFormula::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& AtomicFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " " << AtomicFormula::symbol_to_string.at(symbol()) << " " << *rhs;
	return os;
}


bool ActionEffect::isWellFormed() const {
	auto lhs_var = dynamic_cast<StateVariable::cptr>(lhs);
	auto lhs_fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(lhs);
	return lhs_var || lhs_fluent; // The LHS of the effect must be either a state variable or a fluent function.
}

Atom ActionEffect::apply(const State& state) const {
	return Atom(lhs->interpretVariable(state), rhs->interpret(state));
}

void ActionEffect::computeScope(std::set<VariableIdx>& scope) const {
	// The left hand side of the effect only contributes to the set of relevant variables if it itself
	// is headed by a fluent function containing other state variables.
	if (FluentHeadedNestedTerm::cptr lhs_fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(lhs)) {
		lhs_fluent->computeSubtermScope(scope);
	}
	rhs->computeScope(scope);
}

VariableIdxVector ActionEffect::computeScope() const {
	std::set<VariableIdx> set;
	computeScope(set);
	return VariableIdxVector(set.cbegin(), set.cend());
}

void ActionEffect::computeAffected(std::set<VariableIdx>& affected) const {
	if (auto lhs_var = dynamic_cast<StateVariable::cptr>(lhs)) {
		affected.insert(lhs_var->getValue());
	} else {
		// We necessarily have a LHS term headed by a fluent function
		auto lhs_fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(lhs);
		assert(lhs_fluent);
		throw std::runtime_error("Nested fluent - To be implemented");
	}
}

VariableIdxVector ActionEffect::computeAffected() const {
	std::set<VariableIdx> set;
	computeAffected(set);
	return VariableIdxVector(set.cbegin(), set.cend());
}


std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " := " << *rhs;
	return os;
}


} } } // namespaces
