
#include <problem_info.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {

// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef AtomicFormula::Symbol AFSymbol;
std::vector<std::pair<AFSymbol, std::string>> symbol_to_string_init{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
const boost::container::flat_map<AFSymbol, std::string> AtomicFormula::symbol_to_string(symbol_to_string_init.begin(), symbol_to_string_init.end());
const boost::container::flat_map<std::string, AFSymbol> AtomicFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


//! A small helper
template <typename T>
std::ostream& _printFunction(std::ostream& os, const fs0::ProblemInfo& info, unsigned symbol_id, const std::vector<T*>& subterms) {
	os << info.getFunctionName(symbol_id) << "(";
	for (const T* term:subterms) {
		os << *term << ", ";
	}
	os << ")";
	return os;
}	

void Term::computeScope(VariableIdxVector& scope) const {
	assert(scope.empty());
	std::set<VariableIdx> set;
	computeScope(set);
	scope.insert(scope.end(), set.cbegin(), set.cend());
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
	return _printFunction(os, info, _symbol_id, _subterms);
}

ObjectIdxVector NestedTerm::interpret_subterms(const PartialAssignment& assignment) const {
	ObjectIdxVector interpreted;
	for (Term::cptr subterm:_subterms) {
		interpreted.push_back(subterm->interpret(assignment));
	}
	return interpreted;
}

StaticHeadedNestedTerm::StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
	: NestedTerm(symbol_id, subterms),
	_function(Problem::getCurrentProblem()->getProblemInfo().getFunctionData(_symbol_id).getFunction())
{}
		 
ObjectIdx StaticHeadedNestedTerm::interpret(const PartialAssignment& assignment) const {
	return _function(interpret_subterms(assignment));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const PartialAssignment& assignment) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	VariableIdx variable = info.resolveStateVariable(_symbol_id, interpret_subterms(assignment));
	return assignment.at(variable);
}

void StateVariable::computeScope(std::set<VariableIdx>& scope) const {
	scope.insert(_variable_id);
}

std::ostream& StateVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.getVariableName(_variable_id);
	return os;
}

std::ostream& Constant::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << _value;
	return os;
}








std::ostream& TermSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& TermSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << "<unnamed unprocessed term>";
	return os;
}

Term* NestedTermSchema::process(const ObjectIdxVector& binding) const {
	std::vector<const Term*> st;
	st.reserve(_subterms.size());
	std::vector<ObjectIdx> constant_values;
	for (const TermSchema* unprocessed_subterm:_subterms) {
		const Term* processed = unprocessed_subterm->process(binding);
		st.push_back(processed);
		
		if (const Constant* constant = dynamic_cast<const Constant*>(processed)) {
			constant_values.push_back(constant->getValue());
		}
	}
	
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	if (info.getFunctionData(_symbol_id).isStatic()) {
		return new StaticHeadedNestedTerm(_symbol_id, st);
	} else {
		// If all subterms were constant, and the symbol is fluent, we have a state variable
		if (constant_values.size() == _subterms.size()) {
			VariableIdx id = info.getVariableId(_symbol_id, constant_values);
			return new StateVariable(id);
		}
		return new FluentHeadedNestedTerm(_symbol_id, st);
	}
}


std::ostream& NestedTermSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _printFunction(os, info, _symbol_id, _subterms);
}


Term* ActionSchemaParameter::process(const ObjectIdxVector& binding) const {
	assert(_position < binding.size());
	return new Constant(binding.at(_position));
}

std::ostream& ActionSchemaParameter::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "?" << _position;
	return os;
}

Term* ConstantSchema::process(const ObjectIdxVector& binding) const {
	return new Constant(_value);
}

std::ostream& ConstantSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << _value;
	return os;
}


void AtomicFormula::computeScope(VariableIdxVector& scope) const {
	assert(scope.empty());
	std::set<VariableIdx> set;
	lhs->computeScope(set);
	rhs->computeScope(set);
	scope.insert(scope.end(), set.cbegin(), set.cend());
}

std::ostream& AtomicFormula::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& AtomicFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " " << AtomicFormula::symbol_to_string.at(symbol()) << " " << *rhs;
	return os;
}

AtomicFormulaSchema::cptr AtomicFormulaSchema::create(const std::string& symbol, TermSchema::cptr lhs, TermSchema::cptr rhs) {
	return new AtomicFormulaSchema(AtomicFormula::string_to_symbol.at(symbol), lhs, rhs);
}

std::ostream& AtomicFormulaSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& AtomicFormulaSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " " << AtomicFormula::symbol_to_string.at(symbol) << " " << *rhs;
	return os;
}

std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " := " << *rhs;
	return os;
}

std::ostream& ActionEffectSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionEffectSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " := " << *rhs;
	return os;
}



} } } // namespaces
