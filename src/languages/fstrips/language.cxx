
#include <problem_info.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

#include <typeinfo>

namespace fs0 { namespace language { namespace fstrips {

// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef RelationalFormula::Symbol AFSymbol;
std::vector<std::pair<AFSymbol, std::string>> symbol_to_string_init{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
const std::map<AFSymbol, std::string> RelationalFormula::symbol_to_string(symbol_to_string_init.begin(), symbol_to_string_init.end());
const std::map<std::string, AFSymbol> RelationalFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


//! A helper to interpret a vector of terms
template <typename T>
ObjectIdxVector NestedTerm::interpret_subterms(const std::vector<Term::cptr>& subterms, const T& assignment) {
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

std::ostream& LogicalElement::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

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
	: NestedTerm(symbol_id, subterms)
{}

UserDefinedStaticTerm::UserDefinedStaticTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
	: StaticHeadedNestedTerm(symbol_id, subterms),
	_function(Problem::getCurrentProblem()->getProblemInfo().getFunctionData(symbol_id).getFunction())
{}
		 
ObjectIdx UserDefinedStaticTerm::interpret(const PartialAssignment& assignment) const {
	return _function(interpret_subterms(_subterms, assignment));
}

ObjectIdx UserDefinedStaticTerm::interpret(const State& state) const {
	return _function(interpret_subterms(_subterms, state));
}


void FluentHeadedNestedTerm::computeScope(std::set<VariableIdx>& scope) const {
	// The scope of a term headed by a nested fluent contains the scope of all the subterms
	// plus all possible state variables in which the top-level fluent might result
	NestedTerm::computeScope(scope);
	computeTopLevelScope(scope);
}

void FluentHeadedNestedTerm::computeTopLevelScope(std::set<VariableIdx>& scope) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	const VariableIdxVector& possible_variables = info.resolveStateVariable(_symbol_id);
	scope.insert(possible_variables.cbegin(), possible_variables.cend());
}

ObjectIdx FluentHeadedNestedTerm::interpret(const PartialAssignment& assignment) const {
	return assignment.at(interpretVariable(assignment));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const State& state) const {
	return state.getValue(interpretVariable(state));
}

VariableIdx FluentHeadedNestedTerm::interpretVariable(const PartialAssignment& assignment) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	VariableIdx variable = info.resolveStateVariable(_symbol_id, interpret_subterms(_subterms, assignment));
	return variable;
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

VariableIdxVector AtomicFormula::computeScope() const {
	std::set<VariableIdx> set;
	computeScope(set);
	return VariableIdxVector(set.cbegin(), set.cend());
}

void AtomicFormula::computeScope(std::set<VariableIdx>& scope) const {
	for (const auto subterm:_subterms) subterm->computeScope(scope);
}

bool AtomicFormula::interpret(const PartialAssignment& assignment) const {
	return _satisfied(NestedTerm::interpret_subterms(_subterms, assignment));
}

bool AtomicFormula::interpret(const State& state) const {
	return _satisfied(NestedTerm::interpret_subterms(_subterms, state));
}


RelationalFormula::cptr RelationalFormula::create(RelationalFormula::Symbol symbol, const std::vector<Term::cptr>& subterms) {
	if (symbol == RelationalFormula::Symbol::EQ)  return new EQAtomicFormula(subterms);
	if (symbol == RelationalFormula::Symbol::NEQ) return new NEQAtomicFormula(subterms);
	if (symbol == RelationalFormula::Symbol::LT)  return new LTAtomicFormula(subterms);
	if (symbol == RelationalFormula::Symbol::LEQ) return new LEQAtomicFormula(subterms);
	if (symbol == RelationalFormula::Symbol::GT)  return new GTAtomicFormula(subterms);
	if (symbol == RelationalFormula::Symbol::GEQ) return new GEQAtomicFormula(subterms);
	assert(0);
}



std::ostream& RelationalFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *_subterms[0] << " " << RelationalFormula::symbol_to_string.at(symbol()) << " " << *_subterms[1];
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
		lhs_fluent->computeTopLevelScope(affected);
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


bool NestedTerm::operator==(const Term& other) const {
	auto derived = dynamic_cast<const NestedTerm*>(&other);
	if (!derived
		|| _symbol_id != derived->_symbol_id
		|| _subterms.size() != derived->_subterms.size()) {
		return false;
	}
	
	for (unsigned i = 0; i < _subterms.size(); ++i) {
		if (_subterms[i] != derived->_subterms[i]) return false;
	}
	
	return true;
}

bool StateVariable::operator==(const Term& other) const {
	auto derived = dynamic_cast<const StateVariable*>(&other);
	return derived && _variable_id == derived->_variable_id;
}

bool Constant::operator==(const Term& other) const {
	auto derived = dynamic_cast<const Constant*>(&other);
	return derived && _value == derived->_value;
}


std::size_t NestedTerm::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _symbol_id);
	for (const Term::cptr term:_subterms) {
		boost::hash_combine(hash, term->hash_code());
	}
	return hash;
}

std::size_t StateVariable::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _variable_id);
	return hash;
}

std::size_t Constant::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _value);
	return hash;
}




} } } // namespaces
