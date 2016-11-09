
#include <unordered_set>

#include <applicability/action_managers.hxx>
#include <actions/actions.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <utils/utils.hxx>

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
std::vector<Atom> NaiveApplicabilityManager::computeEffects(const State& state, const GroundAction& action) {
	Atom::vctr atoms;
	for (const fs::ActionEffect* effect:action.getEffects()) {
		if (effect->applicable(state)) {
			atoms.push_back(effect->apply(state));
		}
	}
	return atoms;
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



SmartActionManager::SmartActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints) :
	_actions(actions),
	_state_constraints(process_state_constraints(state_constraints)),
	_vars_affected_by_actions(),
	_vars_relevant_to_constraints(),
	_sc_index()
{
	index_variables(actions, _state_constraints);
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
}

bool
SmartActionManager::applicable(const State& state, const GroundAction& action) const {
	if (!NaiveApplicabilityManager::checkFormulaHolds(action.getPrecondition(), state)) return false;
	
	auto atoms = NaiveApplicabilityManager::computeEffects(state, action);
	if (!NaiveApplicabilityManager::checkAtomsWithinBounds(atoms)) return false; // TODO - THIS SHOULD BE OPTIMIZED
		
	if (!_state_constraints.empty()) { // If we have no constraints, we can spare the cost of further checks
		State next(state, atoms);
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
	return GroundApplicableSet(*this, state);
}



GroundApplicableSet::GroundApplicableSet(const SmartActionManager& manager, const State& state) :
	_manager(manager), _state(state)
{}
	
GroundApplicableSet::Iterator::Iterator(const State& state, const SmartActionManager& manager, unsigned index) :
	_manager(manager),
	_state(state),
	_index(index)
{
	advance();
}

void GroundApplicableSet::Iterator::advance() {
	const std::vector<const GroundAction*>& actions = _manager._actions;
	// std::cout << "Checking applicability "<< std::endl;
	for (unsigned size = actions.size();_index != size; ++_index) {
		// std::cout << "Checking applicability of: " << *_actions[_index] << std::endl;
		if (_manager.applicable(_state, *actions[_index])) { // The action is applicable, break the for loop.
			// std::cout << "Found applicable action: " << *_actions[_index] << std::endl;
			break;
		}
	}
}

const GroundApplicableSet::Iterator& GroundApplicableSet::Iterator::operator++() {
	++_index;
	advance();
	return *this;
}

} // namespaces



