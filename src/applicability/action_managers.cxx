
#include <numeric>
#include <unordered_set>

#include <lapkt/tools/logging.hxx>

#include <utils/system.hxx>

#include <applicability/action_managers.hxx>
#include <actions/actions.hxx>
#include <state.hxx>
#include <problem_info.hxx>
#include <utils/atom_index.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/operations.hxx>
#include <utils/utils.hxx>
#include <utils/printers/vector.hxx>

namespace fs0 {

NaiveApplicabilityManager::NaiveApplicabilityManager(const fs::Formula* state_constraints)
	: _state_constraints(state_constraints) {}

//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
bool NaiveApplicabilityManager::isApplicable(const State& state, const GroundAction& action) const {
	if (!checkFormulaHolds(action.getPrecondition(), state)) return false;

	auto atoms = computeEffects(state, action);
	if (!checkAtomsWithinBounds(atoms)) return false;

	if (!_state_constraints->is_tautology()) { // If we have no constraints, we can spare the cost of creating the new state.
		State next(state, atoms);
		return checkFormulaHolds(_state_constraints, next);
	}
	return true;
}

//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
std::vector<Atom>
NaiveApplicabilityManager::computeEffects(const State& state, const GroundAction& action) {
	std::vector<Atom> atoms;
	computeEffects(state, action, atoms);
	return atoms;
}

void
NaiveApplicabilityManager::computeEffects(const State& state, const GroundAction& action, std::vector<Atom>& atoms) {
	const auto& effects = action.getEffects();
	atoms.clear();
	atoms.reserve(effects.size());
	for (const fs::ActionEffect* effect:effects) {
		if (effect->applicable(state)) {
			atoms.push_back(effect->apply(state));
		}
	}
}

bool NaiveApplicabilityManager::checkFormulaHolds(const fs::Formula* formula, const State& state) {
	return formula->interpret(state);
}

bool NaiveApplicabilityManager::checkAtomsWithinBounds(const std::vector<Atom>& atoms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (const auto& atom:atoms) {
		if (!info.checkValueIsValid(atom)) return false;
	}
	return true;
}



SmartActionManager::SmartActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints, const AtomIndex& tuple_idx, const BasicApplicabilityAnalyzer& analyzer) :
	Base(actions, state_constraints),
	_tuple_idx(tuple_idx),
	_vars_affected_by_actions(),
	_vars_relevant_to_constraints(),
	_sc_index(),
	_app_index(analyzer.getApplicable()),
	_total_applicable_actions(analyzer.total_actions())
{
	index_variables(actions, fs::check_all_atomic_formulas(_state_constraints));

	/*
	// DEBUG
	for (unsigned j = 0; j < _app_index.size(); ++j) {
		const Atom& atom = _tuple_idx.to_atom(j);
		const std::vector<ActionIdx>& tup_actions = _app_index[j];
		LPT_INFO("cout", "Actions potentially applicable for tuple " << atom << ":" << tup_actions.size());
		LPT_INFO("cout", fs0::print::container(tup_actions));
	}
	*/
	LPT_INFO("cout", "A total of " << _total_applicable_actions << " actions were determined to be applicable to at least one atom");
}



void
SmartActionManager::index_variables(const std::vector<const GroundAction*>& actions, const std::vector<const fs::AtomicFormula*>& constraints) {

	// Index the variables affected by each action
	_vars_affected_by_actions.reserve(actions.size());
	for (const GroundAction* action:actions) {
		std::set<VariableIdx> affected;
		fs::ScopeUtils::compute_affected(*action, affected);
		_vars_affected_by_actions.push_back(std::move(affected));
	}

	// Index the variables relevant to each state constraint
	_vars_relevant_to_constraints.reserve(constraints.size());
	for (const fs::AtomicFormula* atom:constraints) {
		std::set<VariableIdx> relevant;
		fs::ScopeUtils::computeFullScope(atom, relevant);
		_vars_relevant_to_constraints.push_back(std::move(relevant));
	}

	// Now create a map A->[C1, C2, ..., Ck] that maps each action index to the state constraints
	// potentially affected by it
	_sc_index.resize(_vars_affected_by_actions.size());
	for (unsigned action = 0; action < _vars_affected_by_actions.size(); ++action) {
		assert(action == actions[action]->getId()); // Just in case
		const std::set<VariableIdx>& affected = _vars_affected_by_actions[action];

		for (unsigned j = 0; j < constraints.size(); ++j) {
			const std::set<VariableIdx>& relevant = _vars_relevant_to_constraints[j];
			if (!Utils::empty_intersection(affected.begin(), affected.end(), relevant.begin(), relevant.end())) {
				// The state constraint is affected by some effect of the action
				_sc_index[action].push_back(constraints[j]);
			}
		}
	}
}

//! A small helper
ObjectIdx _extract_constant_val(const fs::Term* lhs, const fs::Term* rhs) {
	const fs::Constant* _lhs = dynamic_cast<const fs::Constant*>(lhs);
	const fs::Constant* _rhs = dynamic_cast<const fs::Constant*>(rhs);
	assert(_lhs || _rhs);
	return _lhs ? _lhs->getValue() : _rhs->getValue();
}


void
BasicApplicabilityAnalyzer::build() {
	LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [0]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");

	const ProblemInfo& info = ProblemInfo::getInstance();

	_applicable.resize(_tuple_idx.size());
	_rev_applicable.resize(_actions.size());
	_variable_relevance = std::vector<unsigned>(info.getNumVariables(), 0);

	LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() - TupleIdx size: " << _tuple_idx.size());
	LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() - Actions size: " << _actions.size());
	LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() - size of set of atomidx: " << sizeof(std::unordered_set<AtomIdx>));
	LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [1]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
	
	for (unsigned i = 0; i < _actions.size(); ++i) {
		if (i%100==0) {
			LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [it. " << i << "]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
		}
		const GroundAction& action = *_actions[i];
		if (dynamic_cast<const fs::Tautology*>(action.getPrecondition())) { // If there's no precondition, the action is always potentially applicable
			for (auto& app_set:_applicable) app_set.push_back(i);
			continue;
		}

		const fs::Conjunction* precondition = dynamic_cast<const fs::Conjunction*>(action.getPrecondition());
		if (!precondition) { // If the precondition is not a conjunction, we cannot say much, so we consider the action as always potentially applicable
			for (auto& app_set:_applicable) app_set.push_back(i);
			continue;
		}
		
		auto preconditions = fs::check_all_atomic_formulas(precondition->getSubformulae());


		std::set<VariableIdx> referenced; // The state variables already made reference to by some precondition
		for (const fs::AtomicFormula* sub:preconditions) {
			const fs::AtomicFormula* conjunct = dynamic_cast<const fs::AtomicFormula*>(sub);
			if (!conjunct) throw std::runtime_error("Only conjunctions of atoms supported for this type of applicability analyzer");
			
			const fs::RelationalFormula* rel = dynamic_cast<const fs::RelationalFormula*>(conjunct);
			const fs::EQAtomicFormula* eq = dynamic_cast<const fs::EQAtomicFormula*>(conjunct);
			const fs::NEQAtomicFormula* neq = dynamic_cast<const fs::NEQAtomicFormula*>(conjunct);
			unsigned nestedness = fs::nestedness(*conjunct);
			std::vector<VariableIdx> all_relevant = fs::ScopeUtils::computeDirectScope(conjunct);

			// This implements a very rudimentary test that indexes only preconditions of the form X = x or X != x,
			// furthermore assuming that there are no two preconditions making reference to the same state variable
			if (nestedness > 0 || all_relevant.size() != 1 || !(eq || neq)) continue;

			const fs::StateVariable* sv = dynamic_cast<const fs::StateVariable*>(rel->lhs());
			if (!sv) continue;

// 			std::cout << "Processing action #" << i << ": " << action << std::endl;
// 			std::cout << "Processing conjunct: " << *conjunct << std::endl;

			VariableIdx relevant = all_relevant[0];
			const std::vector<ObjectIdx>& values = info.getVariableObjects(relevant);

			if (!referenced.insert(relevant).second) {
				LPT_INFO("cout", "Conjunct \"" << *conjunct << "\" contains a duplicate reference to state variable \"" << info.getVariableName(relevant) << "\"");
				throw std::runtime_error("BasicApplicabilityAnalyzer requires that no two preconditions make reference to the same state variable");
			}

			_variable_relevance[relevant]++;
			
			if (eq) { // Prec is of the form X=x
// 				std::cout << "Precondition: " << *eq << std::endl;
				ObjectIdx value = _extract_constant_val(eq->lhs(), eq->rhs());
				AtomIdx tup = _tuple_idx.to_index(relevant, value);
				
// 				std::cout << "Corresponding Atom: " << _tuple_idx.to_atom(tup) << std::endl;
				
				_applicable[tup].push_back(i);
				_rev_applicable[i].insert(tup);

			} else { // Prec is of the form X!=x
				assert(neq);
// 				std::cout << "Precondition: " << *eq << std::endl;
				ObjectIdx value = _extract_constant_val(neq->lhs(), neq->rhs());
				for (ObjectIdx v2:values) {
					if (v2 != value) {
						AtomIdx tup = _tuple_idx.to_index(relevant, v2);
						_applicable[tup].push_back(i);
						_rev_applicable[i].insert(tup);
					}
				}
			}
		}

		// Now, for those state variables that have _not_ been referenced, the action is potentially applicable no matter what value the state variable takes.
		for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
			if (referenced.find(var) != referenced.end()) continue;

			for (ObjectIdx val:info.getVariableObjects(var)) {
				AtomIdx tup = _tuple_idx.to_index(var, val);
				_applicable[tup].push_back(i);
			}
		}
	}

	_total_actions =  _actions.size();
	LPT_INFO("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [END]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
}


std::vector<ActionIdx> SmartActionManager::compute_whitelist(const State& state) const {
	std::size_t num_vars = state.numAtoms(); // The number of state variables, i.e. of atoms in a state
	assert(num_vars >= 1); // We have at least one state variable

	// We'll store here the indexes of the atoms that cannot prune any action (because all actions are applicable wrt atoms)
	// We want this because if an atom cannot prune any action, then there's no need to intersect anything.
	std::vector<bool> tuples_with_all_actions(num_vars, false);

	// We find the atom in the state that has associated to it the smallest potentially-applicable action set (we'll start intersecting with that one)
	// At the same time, we precompute the indexes of all atoms in the state
	unsigned var_with_min_app_set = 0, min_size = std::numeric_limits<unsigned>::max();
	std::vector<AtomIdx> tuples(num_vars);
	for (unsigned i = 0; i < num_vars; ++i) {
		AtomIdx tup = _tuple_idx.to_index(i, state.getValue(i));
		tuples[i] = tup;
		unsigned s = _app_index[tup].size();

		if (s < min_size) {
			min_size = s;
			var_with_min_app_set = i;
		}

		if (s == _total_applicable_actions) {
			tuples_with_all_actions[i] = true;
		}
	}


	// Partially based on http://stackoverflow.com/a/12882072
	std::vector<ActionIdx> result = _app_index[tuples[var_with_min_app_set]]; // Copy the vector // only return this one, for NRVO to kick in!

	if (num_vars == 1) { // We simply return a copy of the potentially-applicable actions for the only atom
		return result;
	}

	// Otherwise, the state has at least two atoms, so we'll intersect the "whitelists" of all
	// atom states to obtain a final list of action indexes

	std::vector<ActionIdx> buffer; // outside the loop so that we reuse its memory
	for (unsigned i = 0; i < num_vars; ++i) {
		if (i != var_with_min_app_set && !tuples_with_all_actions[i]) {
			AtomIdx tup = tuples[i];
			const auto& candidates = _app_index[tup];
			buffer.clear();
			std::set_intersection(result.begin(), result.end(), candidates.begin(), candidates.end(), std::back_inserter(buffer));
			std::swap(result, buffer);
		}
	}

	return result;
}

bool
SmartActionManager::check_constraints(unsigned applied_action_id, const State& state) const {
	// Check only those constraints that can be affected by the action last applied
	for (const fs::AtomicFormula* constraint:_sc_index[applied_action_id]) {
		if (!constraint->interpret(state)) return false;
	}
	return true;
}


//! A local helper to build a list <0,1,...,size>
std::vector<ActionIdx> _build_all_actions_whitelist(unsigned size) {
	std::vector<ActionIdx> vector(size);
	std::iota(vector.begin(), vector.end(), 0);
	return vector;
}

//! A local helper to ensure the provided state constraints are of the adequate form.
std::vector<const fs::Formula*>
_process_state_constraints(const fs::Formula* state_constraints) {
	if (dynamic_cast<const fs::Tautology*>(state_constraints)) return {};

	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(state_constraints);
	if (!conjunction) throw std::runtime_error("Unsupported State Constraint type");

	return conjunction->getSubformulae();
}


NaiveActionManager::NaiveActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints) :
	_actions(actions),
	_state_constraints(_process_state_constraints(state_constraints)),
	_all_actions_whitelist(_build_all_actions_whitelist(actions.size()))
{}

bool
NaiveActionManager::applicable(const State& state, const GroundAction& action) const {
	if (!NaiveApplicabilityManager::checkFormulaHolds(action.getPrecondition(), state)) return false;

	NaiveApplicabilityManager::computeEffects(state, action, _effects_cache);
	if (!NaiveApplicabilityManager::checkAtomsWithinBounds(_effects_cache)) return false; // TODO - THIS SHOULD BE OPTIMIZED

	if (!_state_constraints.empty()) { // If we have no constraints, we can spare the cost of further checks
		State next(state, _effects_cache);
		return check_constraints(action.getId(), next);
	}
	return true;
}

GroundApplicableSet
NaiveActionManager::applicable(const State& state) const {
	return GroundApplicableSet(*this, state, compute_whitelist(state));
}


bool
NaiveActionManager::check_constraints(unsigned applied_action_id, const State& state) const {
	for (const auto constraint:_state_constraints) { // Check all state constraints
		if (!constraint->interpret(state)) return false;
	}
	return true;
}

} // namespaces
