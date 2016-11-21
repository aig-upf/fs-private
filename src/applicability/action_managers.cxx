
#include <unordered_set>

#include <aptk2/tools/logging.hxx>
#include <applicability/action_managers.hxx>
#include <actions/actions.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
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



SmartActionManager::SmartActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints, const TupleIndex& tuple_idx, const BasicApplicabilityAnalyzer* analyzer) :
	_actions(actions),
	_state_constraints(process_state_constraints(state_constraints)),
	_tuple_idx(tuple_idx),
	_vars_affected_by_actions(),
	_vars_relevant_to_constraints(),
	_sc_index(),
	_app_index(analyzer->getApplicable()),
	_total_applicable_actions(analyzer->total_actions())
{
	index_variables(actions, _state_constraints);
	
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

std::vector<const fs::AtomicFormula*>
SmartActionManager::process_state_constraints(const fs::Formula* state_constraints) {
	if (dynamic_cast<const fs::Tautology*>(state_constraints)) return {};

	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(state_constraints);
	if (!conjunction) throw std::runtime_error("Unsupported State Constraint type");

	return conjunction->getConjuncts();
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
	
	
	// HACK HACK HACK
	const ProblemInfo& info = ProblemInfo::getInstance();
	ObjectIdx ca0 = info.getObjectId("ca0");
	const ExternalI& external = info.get_external();
	for (unsigned j = 0; j < _sc_index.size(); j++) {
		const GroundAction& action = *(actions.at(j));
		if (action.getName() != "transition_arm" && action.getName() != "grasp-object") {
			_sc_index[j].clear();
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

	const ProblemInfo& info = ProblemInfo::getInstance();
	
	_applicable.resize(_tuple_idx.size());
// 	std::vector<std::vector<ActionIdx>> index(_tuple_idx.size());
	
	
	for (unsigned i = 0; i < _actions.size(); ++i) {
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
		
		
		
		std::set<VariableIdx> referenced; // The state variables already made reference to by some precondition
		
		for (const fs::AtomicFormula* conjunct:precondition->getConjuncts()) {
			
			const fs::RelationalFormula* rel = dynamic_cast<const fs::RelationalFormula*>(conjunct);
			const fs::EQAtomicFormula* eq = dynamic_cast<const fs::EQAtomicFormula*>(conjunct);
			const fs::NEQAtomicFormula* neq = dynamic_cast<const fs::NEQAtomicFormula*>(conjunct);
			unsigned nestedness = conjunct->nestedness();
			std::vector<VariableIdx> all_relevant = fs::ScopeUtils::computeDirectScope(conjunct);
			
			// This implements a very rudimentary test that indexes only preconditions of the form X = x or X != x,
			// furthermore assuming that there are no two preconditions making reference to the same state variable
			if (nestedness > 0 || all_relevant.size() != 1 || !(eq || neq)) continue;
			
			const fs::StateVariable* sv = dynamic_cast<const fs::StateVariable*>(rel->lhs());
			if (!sv) continue;
			
// 			std::cout << "Processing conjunct: " << *conjunct << std::endl;
			
			VariableIdx relevant = all_relevant[0];
			const std::vector<ObjectIdx>& values = info.getVariableObjects(relevant);
			
			if (!referenced.insert(relevant).second) {
				throw std::runtime_error("SmartActionManager requires that no two preconditions make reference to the same state variable");
			}
			
			if (eq) { // Prec is of the form X=x
				ObjectIdx value = _extract_constant_val(eq->lhs(), eq->rhs());
				TupleIdx tup = _tuple_idx.to_index(relevant, value);
				_applicable[tup].push_back(i);
				
			} else { // Prec is of the form X!=x
				assert(neq);
				ObjectIdx value = _extract_constant_val(neq->lhs(), neq->rhs());
				for (ObjectIdx v2:values) {
					if (v2 != value) {
						TupleIdx tup = _tuple_idx.to_index(relevant, v2);
						_applicable[tup].push_back(i);
					}
				}
			}
		}
		
		// Now, for those state variables that have _not_ been referenced, the action is potentially applicable no matter what value the state variable takes.
		for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
			if (referenced.find(var) != referenced.end()) continue;
			
			for (ObjectIdx val:info.getVariableObjects(var)) {
				TupleIdx tup = _tuple_idx.to_index(var, val);
				_applicable[tup].push_back(i);
			}
		}
	}
	
	_total_actions =  _actions.size();
}


std::vector<ActionIdx> SmartActionManager::compute_whitelist(const State& state) const {
	const std::vector<ObjectIdx>& values = state.getValues();
	
	assert(values.size() >= 1); // We have at least one state variable
	
// 	std::cout << "Sizes of potentially applicable action vectors: " << std::endl;
	
	std::vector<bool> tuples_with_all_actions(values.size(), false); // We'll store here which tuples don't prune any action (if they don't: no need to intersect!)
	
	// We find the state variable values giving us the smallest potentially-applicable action set
	// and at the same time precompute all tuple indexes of the state atoms.
	TupleIdx min_tuple_idx = 0, min_size = std::numeric_limits<unsigned>::max();
	std::vector<TupleIdx> tuples(values.size());
	for (unsigned i = 0; i < values.size(); ++i) {
		TupleIdx tup = _tuple_idx.to_index(i, values[i]);
		tuples[i] = tup;
		unsigned s = _app_index[tup].size();
		
		if (s < min_size) {
			min_size = s;
			min_tuple_idx = tup;
		}
		
// 		std::cout << s;
		if (s == _total_applicable_actions) {
			tuples_with_all_actions[i] = true;
// 			std::cout << "*";
		}
// 		std::cout <<  ", ";
	}
	
// 	std::cout << std::endl << std::endl << std::endl;
	
	// Partially based on http://stackoverflow.com/a/12882072
	
	std::vector<ActionIdx> result = _app_index[min_tuple_idx]; // Copy the vector // only return this one, for NRVO to kick in!
	
	if (tuples.size() == 1) { // We simply return a copy of the potentially-applicable actions for the only atom
		return result;
	}
	
	// Else we have at least two state variables 
	
	std::vector<ActionIdx> buffer; // outside the loop so that we reuse its memory
	
	for (unsigned j = 0, sz = tuples.size(); j < sz; ++j) {
		TupleIdx tup = tuples[j];
// 		if (tuples_with_all_actions[j]) std::cout << "Smart one!" << std::endl;
		if (tup != min_tuple_idx && !tuples_with_all_actions[j]) {
			const auto& candidates = _app_index[tup];
			buffer.clear();
			std::set_intersection(result.begin(), result.end(), candidates.begin(), candidates.end(), std::back_inserter(buffer));
			std::swap(result, buffer);
		}
	}
	
// 	result.resize(_actions.size()); std::iota(result.begin(), result.end(), 0); // !!!
	return result;
}

bool
SmartActionManager::applicable(const State& state, const GroundAction& action) const {
	if (!NaiveApplicabilityManager::checkFormulaHolds(action.getPrecondition(), state)) return false;
	
	NaiveApplicabilityManager::computeEffects(state, action, _effects_cache);
	if (!NaiveApplicabilityManager::checkAtomsWithinBounds(_effects_cache)) return false; // TODO - THIS SHOULD BE OPTIMIZED
		
	if (!_state_constraints.empty()) { // If we have no constraints, we can spare the cost of further checks
		State next(state, _effects_cache);
		return check_constraints(action.getId(), next);
	}
	return true;
}

bool
SmartActionManager::check_constraints(unsigned action_id, const State& state) const {
	// Check only those constraints that can be affected by the action last applied
	for (const fs::AtomicFormula* constraint:_sc_index[action_id]) {
		if (!constraint->interpret(state)) return false;
	}
	return true;
}

GroundApplicableSet SmartActionManager::applicable(const State& state) const {
	return GroundApplicableSet(*this, state, compute_whitelist(state));
}



GroundApplicableSet::GroundApplicableSet(const SmartActionManager& manager, const State& state, const std::vector<ActionIdx>& action_whitelist) :
	_manager(manager), _state(state), _whitelist(action_whitelist)
{}
	
GroundApplicableSet::Iterator::Iterator(const State& state, const SmartActionManager& manager, const std::vector<ActionIdx>& action_whitelist, unsigned index) :
	_manager(manager),
	_state(state),
	_whitelist(action_whitelist),
	_index(index)
{
// 	std::cout << "Whitelist size: " << _whitelist.size() << " vs. num actions: " << _manager._actions.size() << std::endl;
	advance();
}

void GroundApplicableSet::Iterator::advance() {
	const std::vector<const GroundAction*>& actions = _manager._actions;
	
	
// 	if (_index == 0) {
// 		for(unsigned j = 0; j < _whitelist[_index]; ++j) {
// 			if (_manager.applicable(_state, *actions[j])) {
// 				std::cout << "Action " << *actions[j] << " is applicable!!!" << std::endl;
// 				throw std::runtime_error("WHOCK0");
// 			}
// 		}
// 	}
	
	
	
	
	// std::cout << "Checking applicability "<< std::endl;
// 	for (ActionIdx action_idx:_whitelist) {
	for (unsigned sz = _whitelist.size();_index < sz; ++_index) {
			unsigned action_idx = _whitelist[_index];
// 			if (_index > 0) {
// 				unsigned prev = _whitelist[_index-1], cur = _whitelist[_index];
// 				for(unsigned j = prev+1; j < cur; ++j) {
// 					if (_manager.applicable(_state, *actions[j])) {
// 						std::cout << "Action " << *actions[j] << " is applicable!!!" << std::endl;
// 						throw std::runtime_error("WHOCK");
// 					}
// 				}
// 			}
			
			
		if (_manager.applicable(_state, *actions[action_idx])) { // The action is applicable, break the for loop.
			// std::cout << "Found applicable action: " << *_actions[action_idx] << std::endl;
			return;
		}
	}
	
	
	
// 	if (_index > 0) {
// 		unsigned prev = _whitelist[_index-1];
// 		for(unsigned j = prev+1; j < actions.size(); ++j) {
// 			if (_manager.applicable(_state, *actions[j])) {
// 				std::cout << "Action " << *actions[j] << " is applicable!!!" << std::endl;
// 				throw std::runtime_error("WHOCK2");
// 			}
// 		}
// 	}
}

const GroundApplicableSet::Iterator& GroundApplicableSet::Iterator::operator++() {
	++_index;
	advance();
	return *this;
}

} // namespaces



