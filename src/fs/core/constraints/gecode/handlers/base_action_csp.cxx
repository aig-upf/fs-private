
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/constraints/gecode/handlers/base_action_csp.hxx>
#include <fs/core/constraints/gecode/helper.hxx>
#include <fs/core/constraints/gecode/utils/novelty_constraints.hxx>
#include <lapkt/tools/logging.hxx>
#include <gecode/driver.hh>
#include <fs/core/heuristics/relaxed_plan/rpg_data.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/atom_index.hxx>


namespace fs0::gecode {


BaseActionCSP::BaseActionCSP(const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions)
	: BaseCSP(tuple_index, approximate), _novelty(nullptr), _hmaxsum_priority(Config::instance().useMinHMaxSumSupportPriority()), _use_effect_conditions(use_effect_conditions)
{
}

BaseActionCSP::~BaseActionCSP() {
	delete _novelty;
}


bool BaseActionCSP::init(bool use_novelty_constraint) {
	LPT_DEBUG("translation", "Gecode Action Handler: processing action " << get_action());
	index();
	
	createCSPVariables(use_novelty_constraint);
	Helper::postBranchingStrategy(*_base_csp);
	
// 	LPT_DEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << " (#: no constraints): " << _translator); // Uncomment for extreme debugging

	for (const auto effect:get_effects()) {
		registerEffectConstraints(effect);
	}
	
// 	LPT_DEBUG("translation", "CSP so far consistent? " << (_base_csp.status() != Gecode::SpaceStatus::SS_FAILED) << " (#: type-bound constraints only): " << _translator); // Uncomment for extreme debugging
	
	register_csp_constraints();

	LPT_DEBUG("translation", "Action " << get_action() << " results in CSP handler:" << std::endl << *this);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until a fixpoint
	Gecode::SpaceStatus st = _base_csp->status();
	if (st == Gecode::SpaceStatus::SS_FAILED) return false;
	
	index_scopes(); // This needs to be _after_ the CSP variable registration
	return true;
}


void BaseActionCSP::process(RPGIndex& graph) const {
	log();
	
	GecodeCSP* csp = instantiate(graph);

	if (!csp || !csp->propagate()) { // This colaterally enforces propagation of constraints
		LPT_EDEBUG("heuristic", "The action CSP is locally inconsistent "); // << print::csp(handler->getTranslator(), *csp));
	} else {
		if (_approximate) {  // Check only local consistency
			//compute_approximate_support(csp, _action_idx, rpg, seed);
			// TODO - Unimplemented, but now sure it makes a lot of sense to solve the action CSPs approximately as of now
			throw UnimplementedFeatureException("Approximate support not yet implemented in action CSPs");
		} else { // Solve the CSP completely
			compute_support(csp, graph);
		}
	}
	delete csp;
}


//! In the case of grounded actions and action schemata, we need to retrieve both the atoms and terms
//! appearing in the precondition, _and_ the terms appearing in the effects, except the root LHS atom.
void BaseActionCSP::index() {
	std::vector<const fs::Formula*> conditions{get_precondition()};

	// If using the effect conditions, we'll want to index their components too
	if (_use_effect_conditions) {
		for (const fs::ActionEffect* effect:get_effects()) {
			conditions.push_back(effect->condition());
		}
	}
	
	// Index formula elements
	index_csp_elements(conditions);
	
	// Index the variables IDs that are relevant for the preconditions
	assert(_action_support.empty());
	for (auto term:_all_terms) {
		if (auto casted = dynamic_cast<const fs::StateVariable*>(term)) {
			_action_support.insert(casted->getValue());
		}
	}
	
	// Index effect elements
	for (const fs::ActionEffect* effect:get_effects()) {
		const auto all_terms = fs::all_terms(*effect->rhs());
		_all_terms.insert(all_terms.cbegin(), all_terms.cend());
		
		// As for the LHS of the effect, ATM we only register the LHS subterms (if any)
		if (auto lhs = dynamic_cast<const fs::FluentHeadedNestedTerm*>(effect->lhs())) {
			for (const fs::Term* term:lhs->getSubterms()) {
				auto subterms = fs::all_terms(*term);
				_all_terms.insert(subterms.cbegin(), subterms.cend());
			}
		}
	}
}


void BaseActionCSP::index_scopes() {
	auto effects = get_effects();
	effect_support_variables.resize(effects.size());
	effect_nested_fluents.resize(effects.size());
	effect_rhs_variables.resize(effects.size());
	effect_lhs_variables.resize(effects.size());
	
	_has_nested_lhs = false;
	_has_nested_relevant_terms = false;

	for (unsigned i = 0; i < effects.size(); ++i) {
		// Insert first the variables relevant to the particular effect and only then the variables relevant to the
		// action which were not already inserted
		effect_support_variables[i] = fs::ScopeUtils::computeDirectScope(effects[i]);
		std::set<VariableIdx> effect_support(effect_support_variables[i].begin(), effect_support_variables[i].end());

		std::set_difference(
			_action_support.begin(), _action_support.end(),
			effect_support.begin(), effect_support.end(),
			std::inserter(effect_support_variables[i], effect_support_variables[i].begin()));


		// We index all the nested terms that appear both in the precondition and in the relevant parts of the effect
		fs::ScopeUtils::TermSet nested;
		fs::ScopeUtils::computeIndirectScope(effects[i], nested);
		fs::ScopeUtils::computeIndirectScope(get_precondition(), nested);
		effect_nested_fluents[i] = std::vector<const fs::FluentHeadedNestedTerm*>(nested.cbegin(), nested.cend());
		
		
		effect_rhs_variables[i] = _translator.resolveVariableIndex(effects[i]->rhs());
		
		_has_nested_relevant_terms = _has_nested_relevant_terms || !effect_nested_fluents[i].empty();
		
		if (!fs::flat(*effects[i]->lhs())) {
			_has_nested_lhs = true;
		} else {
			effect_lhs_variables[i] = fs::interpret_variable(*effects[i]->lhs(), PartialAssignment());
		}
	}
	
	if (_has_nested_lhs) {
		effect_lhs_variables = {}; // Just in case
	}
}


void BaseActionCSP::create_novelty_constraint() {
	_novelty = NoveltyConstraint::createFromEffects(_translator, get_precondition(), get_effects());
}

void BaseActionCSP::post_novelty_constraint(GecodeCSP& csp, const RPGIndex& rpg) const {
	if (_novelty) _novelty->post_constraint(csp, rpg);
}

void BaseActionCSP::registerEffectConstraints(const fs::ActionEffect* effect) {
	// Note: we no longer use output variables, etc.
	// Equate the output variable corresponding to the LHS term with the input variable corresponding to the RHS term
	// const Gecode::IntVar& lhs_gec_var = _translator.resolveVariable(effect->lhs(), CSPVariableType::Output, _base_csp);
	// const Gecode::IntVar& rhs_gec_var = _translator.resolveVariable(effect->rhs(), CSPVariableType::Input, _base_csp);
	// Gecode::rel(_base_csp, lhs_gec_var, Gecode::IRT_EQ, rhs_gec_var);
	
	// Impose a bound on the RHS based on the type of the LHS
	if (ProblemInfo::getInstance().isBoundedType(fs::type(*effect->lhs()))) {
		const Gecode::IntVar& rhs_gec_var = _translator.resolveVariable(effect->rhs(), *_base_csp);
		const auto& lhs_bounds = fs::bounds(*effect->lhs());
		Gecode::dom(*_base_csp, rhs_gec_var, lhs_bounds.first, lhs_bounds.second);
	}
}

void BaseActionCSP::compute_support(GecodeCSP* csp, RPGIndex& graph) const {
	LPT_EDEBUG("heuristic", "Computing full support for action " << get_action());
	Gecode::DFS<GecodeCSP> engine(csp);
	unsigned num_solutions = 0;
	while (GecodeCSP* solution = engine.next()) {
		LPT_EDEBUG("heuristic", std::endl << "Processing action CSP solution #"<< num_solutions + 1 << ": " << print::csp(_translator, *solution))
		process_solution(solution, graph);
		++num_solutions;
		delete solution;
	}

	LPT_EDEBUG("heuristic", "Solving the Action CSP completely produced " << num_solutions << " solutions");
}


void BaseActionCSP::process_solution(GecodeCSP* solution, RPGIndex& graph) const {
	PartialAssignment assignment;
	Binding binding;
	if (_has_nested_lhs || _has_nested_relevant_terms) {
		assignment = _translator.buildAssignment(*solution);
		binding = build_binding_from_solution(solution);
	}
	
	// We compute, effect by effect, the atom produced by the effect for the given solution, as well as its supports
	for (unsigned i = 0; i < get_effects().size(); ++i) {
		const fs::ActionEffect* effect = get_effects()[i];
		VariableIdx variable = _has_nested_lhs ? fs::interpret_variable(*effect->lhs(), assignment, binding) : effect_lhs_variables[i];
		object_id value = _translator.resolveValueFromIndex(effect_rhs_variables[i], *solution);
		AtomIdx reached_tuple = _tuple_index.to_index(variable, value);
		LPT_EDEBUG("heuristic", "Processing effect \"" << *effect << "\"");
		if (_hmaxsum_priority) WORK_IN_PROGRESS("This hasn't been adapted yet to the new tuple-based data structures"); // hmax_based_atom_processing(solution, graph, atom, i, assignment, binding);
		else simple_atom_processing(solution, graph, reached_tuple, i, assignment, binding);
	}
}

void BaseActionCSP::simple_atom_processing(GecodeCSP* solution, RPGIndex& graph, AtomIdx tuple, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const {
	
	bool reached = graph.reached(tuple);
	LPT_EDEBUG("heuristic", "Processing effect \"" << *get_effects()[effect_idx] << "\" produces " << (reached ? "repeated" : "new") << " tuple " << tuple);
	
	if (reached) return; // The value has already been reached before
	
	// Otherwise, the value is actually new - we extract the actual support from the solution
	
	// TODO - THE EXTRACTION OF THE SUPPORT SHOULD BE MERGED WITH THE STANDARD PROCEDURE FOR EXTRACTION OF SINGLE EFFECT SUPPORTS.
	// TODO - i.e., should be something like:
	// std::vector<AtomIdx> support = Supports::extract_support(solution, _translator, _tuple_indexes, _necessary_tuples);
	
	std::vector<AtomIdx> support = extract_support_from_solution(solution, effect_idx, assignment, binding);
	graph.add(tuple, get_action_id(solution), std::move(support));
}


std::vector<AtomIdx> BaseActionCSP::extract_support_from_solution(GecodeCSP* solution, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const {
	std::vector<AtomIdx> support;

	// First extract the supports of the "direct" state variables
	for (VariableIdx variable:effect_support_variables[effect_idx]) {
		object_id value = _translator.resolveInputStateVariableValue(*solution, variable);
		support.push_back(_tuple_index.to_index(variable, value));
	}
	
	// Now the support of atoms such as 'clear(b)' that might appear in formulas in non-negated form.
	support.insert(support.end(), _necessary_tuples.begin(), _necessary_tuples.end());
	
	// And finally the support derived from nested terms
	extract_nested_term_support(solution, effect_nested_fluents[effect_idx], assignment, binding, support);

	return support;
}

Binding BaseActionCSP::build_binding_from_solution(const GecodeCSP* solution) const { return Binding::EMPTY_BINDING; }

void BaseActionCSP::extract_nested_term_support(const GecodeCSP* solution, const std::vector<const fs::FluentHeadedNestedTerm*>& nested_terms, const PartialAssignment& assignment, const Binding& binding, std::vector<AtomIdx>& support) const {
	if (nested_terms.empty()) return;

	// And now of the derived state variables. Note that we keep track dynamically (with the 'insert' set) of the actual variables into which
	// the CSP solution resolves to prevent repetitions
	std::set<VariableIdx> inserted;

	
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	for (const fs::FluentHeadedNestedTerm* fluent:nested_terms) {
		VariableIdx variable = info.resolveStateVariable(fluent->getSymbolId(), _translator.resolveValues(fluent->getSubterms(), *solution));
//		VariableIdx variable = fluent->interpretVariable(assignment, binding);
		if (inserted.find(variable) == inserted.end()) { // Don't push twice the support the same atom
			// object_id value = fluent->interpret(assignment, binding);
			
			object_id value = make_object<int>(1); // i.e. assuming that there are no negated atoms on conditions.
			if (!info.isPredicate(fluent->getSymbolId())) {
				value = _translator.resolveValue(fluent, *solution);
			}
			
			support.push_back(_tuple_index.to_index(variable, value));
			inserted.insert(variable);
		}
	}
}

// TODO - This hasn't been adapted yet to the new tuple-based data structures
/* 
void BaseActionCSP::hmax_based_atom_processing(GecodeCSP* solution, RPGIndex& graph, const Atom& atom, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const {
	auto hint = bookkeeping.getInsertionHint(atom);
	LPT_EDEBUG("heuristic", "Effect produces " << (hint.first ? "new" : "repeated") << " atom " << atom);
	
	std::shared_ptr<std::vector<Atom>> support = extract_support_from_solution(solution, effect_idx, assignment, binding);
	
	if (hint.first) { // If the atom is new, we simply insert it
		bookkeeping.add(atom, get_action_id(solution), support, hint.second);
	} else { // Otherwise, we overwrite the previous atom support only as long as it has been reached in the current RPG layer and the sum of h_max values of the new one is lower
		RPGData::AtomSupport& previous_support = hint.second->second;
		
		unsigned layer = std::get<0>(previous_support); // The layer at which the atom had already been achieved
		if (layer < bookkeeping.getCurrentLayerIdx()) return; // Don't overwrite the atom support if it had been reached in a previous layer.
		
		const std::vector<Atom>& previous_support_atoms = *(std::get<2>(previous_support));
		
		if (bookkeeping.compute_hmax_sum(*support) < bookkeeping.compute_hmax_sum(previous_support_atoms)) {
			LPT_EDEBUG("heuristic", "Atom " << atom << " inserted anyway because of lower sum of h_max values");
			previous_support = bookkeeping.createAtomSupport(get_action_id(solution), support); // Simply update the element with the assignment operator
		}
	}
}
*/

} // namespaces
