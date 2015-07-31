
#include <applicability_manager.hxx>
#include <actions/ground_action.hxx>
#include <state.hxx>


namespace fs0 {


ApplicabilityManager::ApplicabilityManager(const std::vector<AtomicFormula::cptr>& state_constraints)
	: _state_constraints(state_constraints) {}
	
//! Return true iff the preconditions of the applicable entity hold.
bool ApplicabilityManager::checkPreconditionsHold(const State& state, const GroundAction& action) const {
	return checkFormulaHolds(action.getConditions(), state);
}

//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
bool ApplicabilityManager::isApplicable(const State& state, const GroundAction& action) const {
	if (!checkPreconditionsHold(state, action)) return false;
	
	if (_state_constraints.size() != 0) { // If we have no constraints, we can spare the cost of creating the new state.
		State next(state, computeEffects(state, action));
		return checkStateConstraintsHold(next);
	}
	return true;
}

bool ApplicabilityManager::checkStateConstraintsHold(const State& state) const {
	return checkFormulaHolds(_state_constraints, state);
}

//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
Atom::vctr ApplicabilityManager::computeEffects(const State& state, const GroundAction& action) const {
	Atom::vctr atoms;
	for (const ActionEffect::cptr effect:action.getEffects()) {
		atoms.push_back(effect->apply(state));
	}
	return atoms;
}

bool ApplicabilityManager::checkFormulaHolds(const std::vector<AtomicFormula::cptr>& formula, const State& state) {
	for (AtomicFormula::cptr atomic:formula) {
		if (!atomic->interpret(state)) return false;
	}
	return true;
}

} // namespaces
