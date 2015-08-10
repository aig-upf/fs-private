
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/logging.hxx>
#include <constraints/registry.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {


void GecodeCSPHandler::registerTermVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerVariables(term, type, csp, translator, variables);
}

void GecodeCSPHandler::registerTermVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) {
	for (const auto term:terms) registerTermVariables(term, type, csp, translator, variables);
}

void GecodeCSPHandler::registerFormulaVariables(const fs::AtomicFormula::cptr condition, Gecode::IntVarArgs& variables) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*condition);
	assert(component_translator);
	component_translator->registerVariables(condition, _base_csp, _translator, variables);
}

void GecodeCSPHandler::registerFormulaVariables(const std::vector<fs::AtomicFormula::cptr>& conditions, Gecode::IntVarArgs& variables) {
	for (const auto condition:conditions) registerFormulaVariables(condition, variables);
}


void GecodeCSPHandler::registerTermConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerConstraints(term, type, csp, translator);
}

void GecodeCSPHandler::registerTermConstraints(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) {
	for (const auto term:terms) registerTermConstraints(term, type,  csp, translator);
}


void GecodeCSPHandler::registerFormulaConstraints(const fs::AtomicFormula::cptr formula) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*formula);
	assert(component_translator);
	component_translator->registerConstraints(formula, _base_csp, _translator);
}

void GecodeCSPHandler::registerFormulaConstraints(const std::vector<fs::AtomicFormula::cptr>& conditions) {
	for (const auto condition:conditions) registerFormulaConstraints(condition);
}

	
} } // namespaces
