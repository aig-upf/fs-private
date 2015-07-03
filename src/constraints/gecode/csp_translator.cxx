
#include <problem.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>

namespace fs0 { namespace gecode {
	
	
void GecodeCSPTranslator::registerCSPVariable(VariableIdx variable, VariableType type, unsigned csp_variable) {
	auto res = _variables.insert(std::make_pair(
		std::make_pair(variable, type),
		csp_variable
	));
	if (!res.second) {
		throw std::runtime_error("Tried to register a CSP variable that already existed: <" + std::to_string(variable) + ">");
	}
}

const Gecode::IntVar& GecodeCSPTranslator::resolveVariable(const SimpleCSP& csp, VariableIdx variable, VariableType type) const {
	auto it = _variables.find(std::make_pair(variable, type));
	if(it == _variables.end()) {
		throw std::runtime_error("Trying to translate a non-existing CSP variable");
	}
	return csp._X[ it->second ];
}

std::ostream& GecodeCSPTranslator::print(std::ostream& os, const SimpleCSP& csp) const {
	os << "CSP with the following variables: " << std::endl;
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	for (auto it:_variables) {
		std::string var_name = info.getVariableName(it.first.first);
		if (it.first.second == VariableType::Output) {
			var_name = var_name + "'"; // We mark output variables with a "'"
		}
		os << "\t" << var_name << ": " << csp._X[it.second] << std::endl;
	}
	return os;
}

} } // namespaces
