
#include <problem.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <relaxed_state.hxx>

namespace fs0 { namespace gecode {
	
	
bool GecodeCSPVariableTranslator::isRegistered(const fs::Term::cptr term, CSPVariableType type) const {
	TranslationKey key(term, type);
	return _registered.find(key) != _registered.end();
}

bool GecodeCSPVariableTranslator::registerConstant(fs::Constant::cptr constant, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TranslationKey key(constant, CSPVariableType::Input);
	
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered
	
	unsigned id = variables.size();
	int value = constant->getValue();
	variables << Gecode::IntVar(csp, value, value);
	
	_registered.insert(it, std::make_pair(key, id)); // Constants are always considered as input variables
	return true;
}

bool GecodeCSPVariableTranslator::registerStateVariable(fs::StateVariable::cptr variable, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TranslationKey key(variable, type);
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered
	
	variables << Helper::createPlanningVariable(csp, variable->getValue());
	
	unsigned id = variables.size()-1;
	_registered.insert(it, std::make_pair(key, id));
	
	// We now cache state variables in different data structures to allow for a more performant subsequent retrieval
	if (type == CSPVariableType::Input) {
		_input_state_variables.insert(std::make_pair(variable->getValue(), id));
	} else if (type == CSPVariableType::Output) {
		_output_state_variables.insert(std::make_pair(variable->getValue(), id));
	}
	return true;
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TranslationKey key(nested, type);
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered
	
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	TypeIdx domain_type = info.getFunctionData(nested->getSymbolId()).getCodomainType();
	
	variables << Helper::createTemporaryVariable(csp, domain_type);
	
	_registered.insert(it, std::make_pair(key, variables.size()-1));
	return true;
}

const Gecode::IntVar& GecodeCSPVariableTranslator::resolveVariable(fs::Term::cptr term, CSPVariableType type, const SimpleCSP& csp) const {
	auto it = _registered.find(TranslationKey(term, type));
	if(it == _registered.end()) {
		throw std::runtime_error("Trying to translate a non-existing CSP variable");
	}
	return csp._X[it->second];
}

Gecode::IntVarArgs GecodeCSPVariableTranslator::resolveVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, const SimpleCSP& csp) const {
	Gecode::IntVarArgs variables;
	for (const Term::cptr term:terms) {
		variables << resolveVariable(term, type, csp);
	}
	return variables;
}


std::ostream& GecodeCSPVariableTranslator::print(std::ostream& os, const SimpleCSP& csp) const {
	os << "Gecode CSP with variables: " << std::endl;
	for (auto it:_registered) {
		os << "\t";
		os << *(it.first.getTerm());
		if (it.first.getType() == CSPVariableType::Output) os << "'"; // We simply mark output variables with a "'"
		os << ": " << csp._X[it.second] << std::endl;
	}
	return os;
}


void GecodeCSPVariableTranslator::updateStateVariableDomains(SimpleCSP& csp, const RelaxedState& layer) const {
	// Iterate over all the input state variables and constrain them accodrding to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		unsigned csp_variable_id = it.second;
		const DomainPtr& domain = layer.getValues(variable);
		Helper::constrainCSPVariable(csp, csp_variable_id, domain);
	}
}



// XXX TO DEPRECATE:
/*
const Gecode::IntVar& GecodeCSPVariableTranslator::resolveVariable(const SimpleCSP& csp, VariableIdx variable, CSPVariableType type) const {
	auto it = _variables.find(std::make_pair(variable, type));
	if(it == _variables.end()) {
		throw std::runtime_error("Trying to translate a non-existing CSP variable");
	}
	return csp._X[ it->second ];
}
*/


Gecode::IntVarArgs GecodeCSPVariableTranslator::resolveFunction(const SimpleCSP& csp, unsigned symbol_id, CSPVariableType type) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	Gecode::IntVarArgs variables;
	for (VariableIdx variable:info.getFunctionData(symbol_id).getStateVariables()) {
// 		variables << resolveVariable(csp, variable, type);
		// TODO - RETHINK
		assert(0);
		
	}
	return variables;
}





} } // namespaces
