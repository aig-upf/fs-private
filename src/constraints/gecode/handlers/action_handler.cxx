
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/scopes.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

GecodeActionCSPHandler::GecodeActionCSPHandler(const GroundAction& action, const std::vector<ActionEffect::cptr>& effects, bool use_novelty_constraint)
	:  GecodeCSPHandler(), _action(action), _effects(effects), _hmaxsum_priority(Config::instance().useMinHMaxSumSupportPriority())
{
	FDEBUG("translation", "Gecode Action Handler: processing action " << _action.getFullName());
	
	setup();
	
	createCSPVariables(use_novelty_constraint);
	Helper::postBranchingStrategy(_base_csp);
	
// 	FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: no constraints): " << _translator); // Uncomment for extreme debugging

	for (const auto effect:_effects) {
		registerEffectConstraints(effect);
	}
	
// 	FDEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << "(#: type-bound constraints only): " << _translator); // Uncomment for extreme debugging
	
	register_csp_constraints();

	FDEBUG("translation", "Action " << _action.getFullName() << " results in CSP handler:" << std::endl << *this);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until a fixpoint
	Gecode::SpaceStatus st = _base_csp.status();
	_unused(st);
	assert(st != Gecode::SpaceStatus::SS_FAILED); // This should never happen, as it means that the action is (statically) unapplicable.
	
	index_scopes(); // This needs to be _after_ the CSP variable registration
}

// If no set of effects is provided, we'll take all of them into account
GecodeActionCSPHandler::GecodeActionCSPHandler(const GroundAction& action, bool use_novelty_constraint)
	:  GecodeActionCSPHandler(action, action.getEffects(), use_novelty_constraint) {}


void GecodeActionCSPHandler::index() {
	auto conditions = _action.getPrecondition()->all_atoms();
	_all_formulas.insert(conditions.cbegin(), conditions.cend());
	
	for (const AtomicFormula::cptr formula:conditions) {
		const auto terms = formula->all_terms();
		_all_terms.insert(terms.cbegin(), terms.cend());
	}
	
	for (const ActionEffect::cptr effect:_effects) {
		const auto terms = effect->rhs()->all_terms();
		_all_terms.insert(terms.cbegin(), terms.cend());
		
		// As for the LHS of the effect, ATM we only register the LHS subterms (if any)
		if (auto lhs = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(effect->lhs())) {
			for (Term::cptr term:lhs->getSubterms()) {
				auto subterms = term->all_terms();
				_all_terms.insert(subterms.cbegin(), subterms.cend());
			}
		}
	}
}


void GecodeActionCSPHandler::index_scopes() {
	auto scope = ScopeUtils::computeActionDirectScope(_action);
	std::set<VariableIdx> action_support(scope.begin(), scope.end());

	effect_support_variables.resize(_effects.size());
	effect_nested_fluents.resize(_effects.size());
	effect_rhs_variables.resize(_effects.size());
	effect_lhs_variables.resize(_effects.size());
	
	_has_nested_lhs = false;

	for (unsigned i = 0; i < _effects.size(); ++i) {
		// Insert first the variables relevant to the particular effect and only then the variables relevant to the
		// action which were not already inserted
		effect_support_variables[i] = ScopeUtils::computeDirectScope(_effects[i]);
		std::set<VariableIdx> effect_support(effect_support_variables[i].begin(), effect_support_variables[i].end());

		std::set_difference(
			action_support.begin(), action_support.end(),
			effect_support.begin(), effect_support.end(),
			std::inserter(effect_support_variables[i], effect_support_variables[i].begin()));

		ScopeUtils::TermSet nested;

		// Order matters - we first insert the nested fluents from the particular effect
		ScopeUtils::computeIndirectScope(_effects[i], nested);
		effect_nested_fluents[i] = std::vector<fs::FluentHeadedNestedTerm::cptr>(nested.cbegin(), nested.cend());


		// And only then the nested fluents from the action preconditions
		// Actually we don't care that much about repetitions between the two sets of terms, since they are checked anyway when
		// transformed into state variables
		nested.clear();
		ScopeUtils::computeIndirectScope(_action.getPrecondition(), nested);
		effect_nested_fluents[i].insert(effect_nested_fluents[i].end(), nested.cbegin(), nested.cend());
		
		effect_rhs_variables[i] = _translator.resolveVariableIndex(_effects[i]->rhs(), CSPVariableType::Input);
		
		if (!_effects[i]->lhs()->flat()) {
			_has_nested_lhs = true;
		} else {
			effect_lhs_variables[i] = _effects[i]->lhs()->interpretVariable({});
		}
	}
	
	if (_has_nested_lhs) {
		effect_lhs_variables = {}; // Just in case
	}
}


void GecodeActionCSPHandler::create_novelty_constraint() {
	_novelty = NoveltyConstraint::createFromEffects(_translator, _action.getPrecondition(), _effects);
}

void GecodeActionCSPHandler::registerEffectConstraints(const fs::ActionEffect::cptr effect) {
	// Note: we no longer use output variables, etc.
	// Equate the output variable corresponding to the LHS term with the input variable corresponding to the RHS term
	// const Gecode::IntVar& lhs_gec_var = _translator.resolveVariable(effect->lhs(), CSPVariableType::Output, _base_csp);
	// const Gecode::IntVar& rhs_gec_var = _translator.resolveVariable(effect->rhs(), CSPVariableType::Input, _base_csp);
	// Gecode::rel(_base_csp, lhs_gec_var, Gecode::IRT_EQ, rhs_gec_var);
	
	// Impose a bound on the RHS based on the type of the LHS
	if (Problem::getInfo().isBoundedType(effect->lhs()->getType())) {
		const Gecode::IntVar& rhs_gec_var = _translator.resolveVariable(effect->rhs(), CSPVariableType::Input, _base_csp);
		const auto& lhs_bounds = effect->lhs()->getBounds();
		Gecode::dom(_base_csp, rhs_gec_var, lhs_bounds.first, lhs_bounds.second);
	}
}


void GecodeActionCSPHandler::compute_support(SimpleCSP* csp, unsigned int actionIdx, RPGData& rpg, const State& seed) const {
	FFDEBUG("heuristic", "Computing full support for action " << _action.getFullName());
	DFS<SimpleCSP> engine(csp);
	unsigned num_solutions = 0;
	while (SimpleCSP* solution = engine.next()) {
		FFDEBUG("heuristic", std::endl << "Processing action CSP solution #"<< num_solutions + 1 << ": " << print::csp(_translator, *solution))
		process_solution(solution, actionIdx, rpg);
		++num_solutions;
		delete solution;
	}

	FFDEBUG("heuristic", "Solving the Action CSP completely produced " << num_solutions << " solutions");
}

void GecodeActionCSPHandler::compute_approximate_support(SimpleCSP* csp, unsigned action_idx, RPGData rpg, const State& seed) {
	// TODO - Unimplemented, but now sure it makes a lot of sense to solve the action CSPs approximately as of now
	throw UnimplementedFeatureException("Approximate support not yet implemented in action CSPs");
}


void GecodeActionCSPHandler::process_solution(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping) const {
	
		PartialAssignment solution_assignment;
		if (_has_nested_lhs) solution_assignment = _translator.buildAssignment(*solution);
		
		// We compute, effect by effect, the atom produced by the effect for the given solution, as well as its supports
		for (unsigned i = 0; i < _effects.size(); ++i) {
			ActionEffect::cptr effect = _effects[i];
			VariableIdx affected = _has_nested_lhs ? effect->lhs()->interpretVariable(solution_assignment) : effect_lhs_variables[i];
			Atom atom(affected, _translator.resolveValueFromIndex(effect_rhs_variables[i], *solution));
			FFDEBUG("heuristic", "Processing effect \"" << *effect << "\"");
			if (_hmaxsum_priority) hmax_based_atom_processing(solution, actionIdx, bookkeeping, atom, i);
			else simple_atom_processing(solution, actionIdx, bookkeeping, atom, i);
		}
}

void GecodeActionCSPHandler::simple_atom_processing(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx) const {
	auto hint = bookkeeping.getInsertionHint(atom);
	FFDEBUG("heuristic", "Effect produces " << (hint.first ? "new" : "repeated") << " atom " << atom);

	if (hint.first) { // The value is actually new - let us compute the supports, i.e. the CSP solution values for each variable relevant to the effect.
		Atom::vctrp support = extract_support_from_solution(solution, effect_idx);
		bookkeeping.add(atom, actionIdx, support, hint.second);
	}
}

void GecodeActionCSPHandler::hmax_based_atom_processing(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx) const {
	auto hint = bookkeeping.getInsertionHint(atom);
	FFDEBUG("heuristic", "Effect produces " << (hint.first ? "new" : "repeated") << " atom " << atom);
	
	Atom::vctrp support = extract_support_from_solution(solution, effect_idx);
	
	if (hint.first) { // If the atom is new, we simply insert it
		bookkeeping.add(atom, actionIdx, support, hint.second);
	} else { // Otherwise, we overwrite the previous atom support only as long as it has been reached in the current RPG layer and the sum of h_max values of the new one is lower
		RPGData::AtomSupport& previous_support = hint.second->second;
		
		unsigned layer = std::get<0>(previous_support); // The layer at which the atom had already been achieved
		if (layer < bookkeeping.getCurrentLayerIdx()) return; // Don't overwrite the atom support if it had been reached in a previous layer.
		
		const std::vector<Atom>& previous_support_atoms = *(std::get<2>(previous_support));
		
		if (bookkeeping.compute_hmax_sum(*support) < bookkeeping.compute_hmax_sum(previous_support_atoms)) {
			FFDEBUG("heuristic", "Atom " << atom << " inserted anyway because of lower sum of h_max values");
			previous_support = bookkeeping.createAtomSupport(actionIdx, support); // Simply update the element with the assignment operator
		}
	}
}


Atom::vctrp GecodeActionCSPHandler::extract_support_from_solution(SimpleCSP* solution, unsigned effect_idx) const {
	
	Atom::vctrp support = std::make_shared<Atom::vctr>();

	// First extract the supports of the "direct" state variables
	for (VariableIdx variable:effect_support_variables[effect_idx]) {
		support->push_back(Atom(variable, _translator.resolveInputStateVariableValue(*solution, variable)));
	}
	
	if (effect_nested_fluents[effect_idx].empty()) return support; // We can spare the creation of the inserted set if no nested fluents are present.

	// And now of the derived state variables. Note that we keep track dynamically (with the 'insert' set) of the actual variables into which
	// the CSP solution resolves to prevent repetitions
	std::set<VariableIdx> inserted;

	for (auto fluent:effect_nested_fluents[effect_idx]) {
		const NestedFluentData& nested_translator = getNestedFluentTranslator(fluent).getNestedFluentData();
		VariableIdx variable = nested_translator.resolveStateVariable(*solution);

		if (inserted.find(variable) == inserted.end()) { // Don't push twice to the support the same atom
			ObjectIdx value = _translator.resolveValue(fluent, CSPVariableType::Input, *solution);
			support->push_back(Atom(variable, value));
			inserted.insert(variable);
		}
	}
	
	return support;
}

} } // namespaces
