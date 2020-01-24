
#include <fs/core/state.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/constraints/gecode/handlers/base_csp.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/constraints/registry.hxx>
#include <fs/core/constraints/gecode/language_translators.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/constraints/gecode/extensions.hxx>
#include <fs/core/utils/utils.hxx>


namespace fs0::gecode {

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
BaseCSP::registerFormulaVariables(const fs::Formula* condition, CSPTranslator& translator) {
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
BaseCSP::registerFormulaConstraints(const fs::Formula* formula, CSPTranslator& translator) {
	auto component_translator = LogicalComponentRegistry::instance().getGecodeTranslator(*formula);
	assert(component_translator);
	component_translator->registerConstraints(formula, translator);
}

//! A helper
template <typename T>
GecodeCSP*
_instantiate(const GecodeCSP& csp,
			 const AtomIndex& tuple_index,
			 const CSPTranslator& translator,
			 const std::vector<ExtensionalConstraint>& extensional_constraints,
			 const T& layer) {
	auto* clone = static_cast<GecodeCSP*>(csp.clone());
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
_instantiate(const GecodeCSP& csp,
			 const AtomIndex& tuple_index,
			 const CSPTranslator& translator,
			 const std::vector<ExtensionalConstraint>& extensional_constraints,
			 const State& state,
			 const StateBasedExtensionHandler& handler) {
	auto* clone = static_cast<GecodeCSP*>(csp.clone());
	translator.updateStateVariableDomains(*clone, state);

	for (const ExtensionalConstraint& constraint:extensional_constraints) {
		if (!constraint.update(*clone, translator, handler.retrieve(constraint.get_term()->getSymbolId()))) {
			delete clone;
			return nullptr;
		}
	}

	return clone;
}


GecodeCSP*
BaseCSP::instantiate_wo_novelty(const RPGIndex& graph) const {
	if (_failed) return nullptr;
	GecodeCSP* csp = _instantiate(*_base_csp, _tuple_index, _translator, _extensional_constraints, graph);
	return csp;
}

GecodeCSP*
BaseCSP::instantiate(const RPGIndex& graph) const {
	if (_failed) return nullptr;
	GecodeCSP* csp = _instantiate(*_base_csp, _tuple_index, _translator, _extensional_constraints, graph);
	if (!csp) return csp; // The CSP was detected unsatisfiable even before propagating anything
	
	// Post the novelty constraint
	post_novelty_constraint(*csp, graph);
	
	return csp;
}

GecodeCSP*
BaseCSP::instantiate(const State& state, const StateBasedExtensionHandler& handler) const {
	if (_failed) return nullptr;
	return _instantiate(*_base_csp, _tuple_index, _translator, _extensional_constraints, state, handler);
}

void
BaseCSP::register_csp_variables() {
	const ProblemInfo& info = ProblemInfo::getInstance();
	const Config& config = Config::instance();

	// Register reification variables necessary e.g. for disjunctions
	for (const auto* a:_reified_atoms) _translator.registerReifiedAtom(a);


	// Register all CSP variables that arise from the logical terms
	for (const auto term:_all_terms) {
		if (const auto * fluent = dynamic_cast<const fs::FluentHeadedNestedTerm*>(term)) {
			unsigned symbol_id = fluent->getSymbolId();
			bool is_predicate = info.isPredicate(symbol_id);

			// Fake fluent-headed terms that are indeed predicates have already been dealt with in method
			// `index_csp_elements`, so this should not be a predicate
			assert (!is_predicate);

			if (config.getOption("element_constraint", true) && _counter.symbol_requires_element_constraint(symbol_id) && !is_predicate) {
				LPT_DEBUG("translation", "Term \"" << *fluent << "\" will be translated into an element constraint");
				NestedFluentElementTranslator tr(fluent);
				tr.register_variables(_translator);
				_nested_fluent_translators.push_back(std::move(tr));
				_nested_fluent_translators_idx.insert(std::make_pair(fluent, _nested_fluent_translators.size() - 1));
			}
			
			else {
				// In order to model an actual term f(x), we'll need an extra CSP variable to model the f-application,
				// i.e. to model it as an extensional constraint that corresponds to the predicate f(x, z)
				_extensional_constraints.emplace_back(fluent, _tuple_index, is_predicate, false);
				_translator.registerNestedTerm(fluent);
				LPT_DEBUG("translation", "Term \"" << *fluent << "\" will be translated into an extensional constraint");
			}
			
		} else if (auto statevar = dynamic_cast<const fs::StateVariable*>(term)) {
			_translator.registerInputStateVariable(statevar->getValue());
		}
		
		else {
// 			std::cout << "Registering term: " << *term << std::endl;
			registerTermVariables(term,  _translator);
		}
	}
	
	// Register any possible CSP variable that arises from the logical formulas
	for (auto condition:_all_formulas) {
		registerFormulaVariables(condition, _translator);
	}
}

void
BaseCSP::register_csp_constraints() {
// 	unsigned i = 0; _unused(i);

	// Register the constraints corresponding to the formula disjuncts
//    std::cout << "Registering DISJUNCTS" << std::endl;
	for (const auto* disjunct:_disjunctions) {
//        std::cout << "Registering disjunct: " << *disjunct << std::endl;
		registerFormulaConstraints(disjunct, _translator);
	}

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
//		std::cout << "Registering variables for formula: " << *condition << std::endl;
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
BaseCSP::index_csp_elements(const std::vector<const fs::Formula*>& conditions) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::unordered_set<const fs::Term*> inserted_terms;
	std::unordered_set<const fs::Formula*> inserted_conditions;
	std::unordered_set<AtomIdx> true_tuples;


	// Collect all atoms and terms in the different formulas
	std::vector<const fs::Term*> terms;
	std::vector<const fs::AtomicFormula*> all_atoms;
	for (const auto* c:conditions) {
		terms = Utils::merge(terms, fs::all_terms(*c));
		all_atoms = Utils::merge(all_atoms, fs::all_atoms(*c));
	}


	// Collect all disjunctions
	for (const auto* c:conditions) {
		_disjunctions = Utils::merge(_disjunctions,
									Utils::filter_by_type<const fs::Disjunction*>(fs::all_nodes(*c)));
	}

	// All atoms that are children of some disjunction will have to be posted as reified constraints.
	for (const auto* c:_disjunctions) {
		auto tmp = fs::all_atoms(*c);
		_reified_atoms.insert(tmp.cbegin(), tmp.cend());
	}



	// TODO - REFACTOR THIS MESS
	// All this hacky preprocessing aims mainly at undoing another hacky preprocessing done at
	// the Python parsing level, where atoms such as "clear(b)" are transformed into "clear(b)=1",
	// where clear(b) is understood as a (binary) term. This is there for historical reasons,
	// and needs to be corrected in the Python code before cleaning up the mess here.
	//
	// Note that the mess here also does a few other things, but these should be delegated into
	// the individual language translator components, and the whole translation process be performed
	// recursively. This would allow us to deal with disjunctions, etc., more elegantly.
	//
	// Note also that this mess is entangled with the one in BaseCSP::register_csp_variables
	for (auto condition:all_atoms) {
		if (auto relational = dynamic_cast<const fs::RelationalFormula*>(condition)) {
			
			if (relational->symbol() == fs::RelationalFormula::Symbol::EQ) {
				
				const fs::Term* candidate = relational->lhs(); // TODO - CHECK SYMMETRICALLY FOR RHS()!!
				
				auto statevar = dynamic_cast<const fs::StateVariable*>(candidate);
				auto nested_fluent = dynamic_cast<const fs::FluentHeadedNestedTerm*>(candidate);

				
				if (statevar || nested_fluent) {
					const fs::FluentHeadedNestedTerm* origin = statevar ? statevar->getOrigin() : nested_fluent;

					if (info.isPredicate(origin->getSymbolId())) {
						// e.g. we had a condition clear(b) = 1, which we'll want to transform into an extensional constraint on the
						// CSP variable representing the constant 'b' wrt the extension given by the clear predicate
						
						auto value = dynamic_cast<const fs::Constant*>(relational->rhs());
						assert(value);
						auto intval = int(value->getValue());
						assert(intval == 1 || intval == 0);

						// 24.01.2020 We revert back to dealing with predicative state variables in the standard way,
						// i.e. through an extra CSP constraint instead of through extensional constraints.
						// Perhaps the way to go would still be using extensional constraints, but at the moment it's
						// buggy for negated nullary atoms, and would require extensive changes to fix that. The
						// clean way to refactor this would be to generate the CSP at preprocessing, e.g. at the Python
						// level, exploiting the ease of term and formula manipulation that we have in there, and then
						// just serialize the CSP we want to post and post it here in the backend. That serialization
						// shouldn't be that complicated, we don't require any nested structure there, just a standard
						// flat CSP: variables, constraints, domains, and their correspondence with the planning terms
						// and formulas.
						if (statevar) {
							// Mark the state variable to allow the later support recovery
							// 24.01.2020 Not even sure what this is used for
							true_tuples.insert(_tuple_index.to_index(statevar->getValue(), make_object<bool>(true)));
						} else {
							// We have a non-ground atom. We want to deal with it through an extensional constraint.
							// Mark the condition as inserted, so we do not need to insert it again!
							inserted_conditions.insert(relational);
							// For _predicate_ state variables, we don't need an actual CSP variable modeling it, so we spare it.
							inserted_terms.insert(candidate);
							// We'll have one extensional constraint per predicate appearing on the condition / formula.
							_extensional_constraints.emplace_back(origin, _tuple_index, true, intval==0);
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

	// Insert into `_all_terms` any term collected in `terms` which is not in `inserted_terms`, i.e., which is
	// not a state variable
	std::copy_if(terms.begin(), terms.end(), std::inserter(_all_terms, _all_terms.begin()),
				 [&inserted_terms] (const fs::Term* term) { return inserted_terms.find(term) == inserted_terms.end(); });

	// Insert into all_formulas and all_terms all those elements in 'conditions' or 'terms' which have not been
	// deemed to deserve a particular treatment such as the one given to e.g. a condition "clear(b) = 1",
	// or those which need reification
	for (const auto& atom:all_atoms) {
		if (inserted_conditions.find(atom) != inserted_conditions.end()) continue;
//        if (_reified_atoms.find(atom) != _reified_atoms.end()) continue;
		_all_formulas.insert(atom);

	}

//	std::cout << "All formulas: " << std::endl;
//	for (const auto* f:_all_formulas) std::cout << *f << std::endl;
//    std::cout << "reified atoms: " << std::endl;
//    for (const auto* f:_reified_atoms) std::cout << *f << std::endl;


	assert(_necessary_tuples.empty());
	_necessary_tuples.insert(_necessary_tuples.end(), true_tuples.begin(), true_tuples.end());
}


} // namespaces
