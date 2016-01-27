
#include <constraints/gecode/handlers/base_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <utils/printers/vector.hxx>
#include <constraints/registry.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <gecode/driver.hh>
#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 { namespace gecode {

BaseCSPHandler::~BaseCSPHandler() {
	if (_novelty) delete _novelty;
}
	
void BaseCSPHandler::init(const RPGData* bookkeeping) {
	_base_csp.init(MinHMaxValueSelector(&_translator, bookkeeping));
}

void BaseCSPHandler::registerTermVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerVariables(term, type, translator);
}

void BaseCSPHandler::registerFormulaVariables(const fs::AtomicFormula::cptr condition, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*condition);
	assert(component_translator);
	component_translator->registerVariables(condition, translator);
}

void BaseCSPHandler::registerTermConstraints(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerConstraints(term, type, translator);
}


void BaseCSPHandler::registerFormulaConstraints(const fs::AtomicFormula::cptr formula, GecodeCSPVariableTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*formula);
	assert(component_translator);
	component_translator->registerConstraints(formula, translator);
}

SimpleCSP::ptr BaseCSPHandler::instantiate_csp(const GecodeRPGLayer& layer) const {
	SimpleCSP* csp = static_cast<SimpleCSP::ptr>(_base_csp.clone());
	_translator.updateStateVariableDomains(*csp, layer);
	
	// Post the novelty constraint
	if (_novelty) _novelty->post_constraint(*csp, layer);
	
	return csp;
}

SimpleCSP::ptr BaseCSPHandler::instantiate_csp(const State& state) const {
	SimpleCSP* csp = static_cast<SimpleCSP::ptr>(_base_csp.clone());
	_translator.updateStateVariableDomains(*csp, state);
	return csp;
}

const NestedFluentElementTranslator& BaseCSPHandler::getNestedFluentTranslator(fs::FluentHeadedNestedTerm::cptr fluent) const { 
	auto it = _nested_fluent_translators_idx.find(fluent);
	assert(it != _nested_fluent_translators_idx.end());
	const NestedFluentElementTranslator& translator = _nested_fluent_translators.at(it->second);
	assert(*translator.getTerm() == *fluent);
	return translator;
}
	
void BaseCSPHandler::setup() {
	index();
	count_variables();
}

void BaseCSPHandler::count_variables() {
	for (fs::Term::cptr term:_all_terms) {
		if (auto sv = dynamic_cast<fs::StateVariable::cptr>(term)) _counter.count_direct(sv->getValue());
		else if (auto fluent = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term)) {
			// Count all state variables derived from the nested fluent
			for (nested_fluent_iterator it(fluent); !it.ended(); ++it) _counter.count_derived(it.getDerivedStateVariable());
		}
	}
}

void BaseCSPHandler::register_csp_variables() {
	//! Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		if (fs::FluentHeadedNestedTerm::cptr fluent = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term)) {
			NestedFluentElementTranslator tr(fluent, _counter);
			tr.register_variables(CSPVariableType::Input, _translator);
			_nested_fluent_translators.push_back(std::move(tr));
			_nested_fluent_translators_idx.insert(std::make_pair(fluent, _nested_fluent_translators.size() - 1));
			
			
		} else if (auto sv = dynamic_cast<fs::StateVariable::cptr>(term)) {
			VariableIdx variable = sv->getValue();
			_translator.registerInputStateVariable(variable, true, _counter.is_nullable(variable));
		}
		
		else {
			registerTermVariables(term, CSPVariableType::Input, _translator);
		}
	}
	
	//! Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) registerFormulaVariables(condition, _translator);
}

void BaseCSPHandler::register_csp_constraints() {
	unsigned i = 0;
	_unused(i);
	
	for (NestedFluentElementTranslator& tr:_nested_fluent_translators) {
		tr.register_constraints(CSPVariableType::Input, _translator);
// 		FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *tr.getTerm() << "): " << _translator); // Uncomment for extreme debugging
	}
	
	
	//! Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		
		// These types of term do not require a custom translator
		if (dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term) ||
			dynamic_cast<fs::StateVariable::cptr>(term)) {
			continue;
		}
		
		registerTermConstraints(term, CSPVariableType::Input, _translator);
// 		FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *term << "): " << _translator); // Uncomment for extreme debugging
	}
	
	//! Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) {
		registerFormulaConstraints(condition, _translator);
// 		FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *condition << "): " << _translator); // Uncomment for extreme debugging		
	}
}

std::ostream& BaseCSPHandler::print(std::ostream& os, const SimpleCSP& csp) const {
	return _translator.print(os, csp);
}

void BaseCSPHandler::createCSPVariables(bool use_novelty_constraint) {
	register_csp_variables();
	
	if (use_novelty_constraint) {
		create_novelty_constraint();
	}
	_translator.perform_registration();
}

} } // namespaces
