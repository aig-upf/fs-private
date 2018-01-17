
#include <fs/core/problem_info.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/constraints/gecode/handlers/base_csp.hxx>
#include <fs/core/constraints/gecode/helper.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_data.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/constraints/registry.hxx>
#include <gecode/driver.hh>
#include <fs/core/constraints/gecode/translators/component_translator.hxx>
#include <fs/core/utils/config.hxx>


namespace fs0 { namespace gecode {

BaseCSP::BaseCSP(const AtomIndex& tuple_index, bool approximate) :
	_base_csp(new GecodeCSP()),
	_failed(false),
	_approximate(approximate),
	_translator(*_base_csp),
	_tuple_index(tuple_index)
{}

void
BaseCSP::update_csp(std::unique_ptr<GecodeCSP>&& csp) { 
	_base_csp = std::move(csp);
}


void
BaseCSP::registerTermVariables(const fs::Term* term, CSPTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerVariables(term, translator);
}

void
BaseCSP::registerFormulaVariables(const fs::AtomicFormula* condition, CSPTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*condition);
	assert(component_translator);
	component_translator->registerVariables(condition, translator);
}

void
BaseCSP::registerTermConstraints(const fs::Term* term, CSPTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*term);
	assert(component_translator);
	component_translator->registerConstraints(term, translator);
}


void
BaseCSP::registerFormulaConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*formula);
	assert(component_translator);
	component_translator->registerConstraints(formula, translator);
}

//! A helper
template <typename T>
GecodeCSP*
_instantiate(const GecodeCSP& csp,
						   const CSPTranslator& translator,
						   const std::vector<ExtensionalConstraint>& extensional_constraints,
						   const T& layer) {
	GecodeCSP* clone = static_cast<GecodeCSP*>(csp.clone());
	translator.updateStateVariableDomains(*clone, layer);
	for (const ExtensionalConstraint& constraint:extensional_constraints) {
		if (!constraint.update(*clone, translator, layer)) {
			delete clone;
			return nullptr;
		}
	}
	return clone;
}

GecodeCSP*
BaseCSP::instantiate_wo_novelty(const RPGIndex& graph) const {
	if (_failed) return nullptr;
	GecodeCSP* csp = _instantiate(*_base_csp, _translator, _extensional_constraints, graph);
	return csp;
}

GecodeCSP*
BaseCSP::instantiate(const RPGIndex& graph) const {
	if (_failed) return nullptr;
	GecodeCSP* csp = _instantiate(*_base_csp, _translator, _extensional_constraints, graph);
	if (!csp) return csp; // The CSP was detected unsatisfiable even before propagating anything
	
	// Post the novelty constraint
	post_novelty_constraint(*csp, graph);
	
	return csp;
}

GecodeCSP*
BaseCSP::instantiate(const State& state) const {
	if (_failed) return nullptr;
	return _instantiate(*_base_csp, _translator, _extensional_constraints, state);
}


void
BaseCSP::register_csp_variables() {
	const ProblemInfo& info = ProblemInfo::getInstance();
	const Config& config = Config::instance();
	
	//! Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		if (const auto * fluent = dynamic_cast<const fs::FluentHeadedNestedTerm*>(term)) {
			unsigned symbol_id = fluent->getSymbolId();
			bool is_predicate = info.isPredicate(symbol_id);
			
			if (config.getOption("element_constraint") && _counter.symbol_requires_element_constraint(symbol_id) && !is_predicate) {
				LPT_DEBUG("translation", "Term \"" << *fluent << "\" will be translated into an element constraint");
				NestedFluentElementTranslator tr(fluent);
				tr.register_variables(_translator);
				_nested_fluent_translators.push_back(std::move(tr));
				_nested_fluent_translators_idx.insert(std::make_pair(fluent, _nested_fluent_translators.size() - 1));
			}
			
			else {
				
				_extensional_constraints.emplace_back(fluent, _tuple_index, is_predicate);
				if (!is_predicate) { // If the term is indeed a term and not a predicate, we'll need an extra CSP variable to model it.
					_translator.registerNestedTerm(fluent);
					LPT_DEBUG("translation", "Term \"" << *fluent << "\" will be translated into an extensional constraint");
				} else {
					LPT_DEBUG("translation", "Atom \"" << *fluent << "\" will be translated into an extensional constraint");
				}
			}
			
		} else if (auto statevar = dynamic_cast<const fs::StateVariable*>(term)) {
			_translator.registerInputStateVariable(statevar->getValue());
		}
		
		else {
// 			std::cout << "Registering term: " << *term << std::endl;
			registerTermVariables(term,  _translator);
		}
	}
	
	//! Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) registerFormulaVariables(condition, _translator);
}

void
BaseCSP::register_csp_constraints() {
// 	unsigned i = 0; _unused(i);
	
	//! Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		
		// These types of term do not require a custom translator
		if (dynamic_cast<const fs::FluentHeadedNestedTerm*>(term) ||
			dynamic_cast<const fs::StateVariable*>(term)) {
			continue;
		}
		
		registerTermConstraints(term, _translator);
// 		LPT_DEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *term << "): " << _translator); // Uncomment for extreme debugging
	}
	
	//! Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) {
		registerFormulaConstraints(condition, _translator);
// 		LPT_DEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *condition << "): " << _translator); // Uncomment for extreme debugging		
	}
	
	for (ExtensionalConstraint& constraint:_extensional_constraints) {
		constraint.register_constraints(_translator);
	}
	
	for (NestedFluentElementTranslator& tr:_nested_fluent_translators) {
		tr.register_constraints(_translator);
		// FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: " << i++ << ", what: " << *tr.getTerm() << "): " << _translator); // Uncomment for extreme debugging
	}
}

std::ostream&
BaseCSP::print(std::ostream& os, const GecodeCSP& csp) const {
	return _translator.print(os, csp);
}

void
BaseCSP::createCSPVariables(bool use_novelty_constraint) {
	register_csp_variables();
	
	if (use_novelty_constraint) {
		create_novelty_constraint();
	}
	_translator.perform_registration();
}

void
BaseCSP::index_formula_elements(const std::vector<const fs::AtomicFormula*>& conditions, const std::vector<const fs::Term*>& terms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::unordered_set<const fs::Term*> inserted_terms;
	std::unordered_set<const fs::AtomicFormula*> inserted_conditions;
	std::unordered_set<AtomIdx> true_tuples;
	
	
	for (auto condition:conditions) {
		if (auto relational = dynamic_cast<const fs::RelationalFormula*>(condition)) {
			
			if (relational->symbol() == fs::RelationalFormula::Symbol::EQ) {
				
				const fs::Term* candidate = relational->lhs(); // TODO - CHECK SYMMETRICALLY FOR RHS()!!
				
// 				auto nested = dynamic_cast<const fs::NestedTerm*>(candidate);
				auto statevar = dynamic_cast<const fs::StateVariable*>(candidate);
				auto nested_fluent = dynamic_cast<const fs::FluentHeadedNestedTerm*>(candidate);

				
				if (statevar || nested_fluent) {
					const fs::FluentHeadedNestedTerm* origin = statevar ? statevar->getOrigin() : nested_fluent;
					unsigned symbol_id = origin->getSymbolId();
					
					if (info.isPredicate(symbol_id)) {
						// e.g. we had a condition clear(b) = 1, which we'll want to transform into an extensional constraint on the
						// CSP variable representing the constant 'b' wrt the extension given by the clear predicate
						
						auto value = dynamic_cast<const fs::Constant*>(relational->rhs());
						if (!value || int(value->getValue()) != 1) {
							throw UnimplementedFeatureException("Only non-negated relational atoms are supported ATM");
						}
						
						// Mark the condition as inserted, so we do not need to insert it again!
						inserted_conditions.insert(condition);
						
						if (statevar) {
							// For _predicate_ state variables, we don't need an actual CSP variable modeling it, so we spare it.
							inserted_terms.insert(candidate);
							
							// We'll have one extensional constraint per predicate appearing on the condition / formula.
							_extensional_constraints.emplace_back(origin, _tuple_index, true);
							
							// Insert subterms properly - TODO - Perhaps StateVariable::all_terms should already return these terms?
							for (auto term:origin->getSubterms()) {
								auto tmp = fs::all_terms(*term);
								_all_terms.insert(tmp.cbegin(), tmp.cend()); // TODO These should go away once we deal with constants separately
							}
							
							// Mark the state variable to allow the later support recovery
							true_tuples.insert(_tuple_index.to_index(statevar->getValue(), make_object<bool>(true)));
						}
					}
				}
			}
		}
	}
	
	for (auto term:terms) {
		if (auto sv = dynamic_cast<const fs::StateVariable*>(term)) _counter.count_flat(sv->getOrigin()->getSymbolId());
		else if (auto fluent = dynamic_cast<const fs::FluentHeadedNestedTerm*>(term)) _counter.count_nested(fluent->getSymbolId());
	}
	
	// Insert into all_formulas and all_terms all those elements in 'conditions' or 'terms' which have not been 
	// deemed to deserve a particular treatment such as the one given to e.g. a condition "clear(b) = 1"
	std::copy_if(conditions.begin(), conditions.end(), std::inserter(_all_formulas, _all_formulas.begin()),
				 [&inserted_conditions] (const fs::AtomicFormula* atom) { return inserted_conditions.find(atom) == inserted_conditions.end(); });
	
	std::copy_if(terms.begin(), terms.end(), std::inserter(_all_terms, _all_terms.begin()),
				 [&inserted_terms] (const fs::Term* term) { return inserted_terms.find(term) == inserted_terms.end(); });
	
	assert(_necessary_tuples.empty());
	_necessary_tuples.insert(_necessary_tuples.end(), true_tuples.begin(), true_tuples.end());
}


} } // namespaces
