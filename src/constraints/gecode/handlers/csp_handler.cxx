
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <constraints/registry.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

void GecodeCSPHandler::init() {
	_base_csp.init(&_translator);
}

void GecodeCSPHandler::registerTermVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerVariables(term, type, translator);
}

void GecodeCSPHandler::registerTermVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	for (const auto term:terms) registerTermVariables(term, type, translator);
}

void GecodeCSPHandler::registerFormulaVariables(const fs::AtomicFormula::cptr condition) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*condition);
	assert(component_translator);
	component_translator->registerVariables(condition, _translator);
}

void GecodeCSPHandler::registerFormulaVariables(const std::vector< AtomicFormula::cptr >& conditions) {
	for (const auto condition:conditions) registerFormulaVariables(condition);
}


void GecodeCSPHandler::registerTermConstraints(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerConstraints(term, type, translator);
}

void GecodeCSPHandler::registerTermConstraints(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	for (const auto term:terms) registerTermConstraints(term, type,  translator);
}


void GecodeCSPHandler::registerFormulaConstraints(const fs::AtomicFormula::cptr formula) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*formula);
	assert(component_translator);
	component_translator->registerConstraints(formula, _translator);
}

void GecodeCSPHandler::registerFormulaConstraints(const std::vector<fs::AtomicFormula::cptr>& conditions) {
	for (const auto condition:conditions) registerFormulaConstraints(condition);
}

SimpleCSP::ptr GecodeCSPHandler::instantiate_csp(const GecodeRPGLayer& layer) const {
	SimpleCSP* csp = static_cast<SimpleCSP::ptr>(_base_csp.clone());
	_translator.updateStateVariableDomains(*csp, layer);
	
	// Post the novelty constraint
	if (_novelty) _novelty->post_constraint(*csp, layer);
	
	return csp;
}


} } // namespaces
