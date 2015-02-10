
#include <standard_applicability_manager.hxx>
#include <actions.hxx>


namespace fs0 {


StandardApplicabilityManager::StandardApplicabilityManager(const State& state, const ScopedConstraint::vcptr& constraints)
	: _state(state), stateConstraints(constraints) {}
	
StandardApplicabilityManager::StandardApplicabilityManager(const StandardApplicabilityManager& other)
	: _state(other._state), stateConstraints(other.stateConstraints) {}

//! Return true iff the preconditions of the applicable entity hold.
bool StandardApplicabilityManager::checkPreconditionsHold(const Action& action) const {
	for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
		if (!constraint->isSatisfied(_state))
			return false;
	}
	return true;
}

//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
bool StandardApplicabilityManager::isApplicable(const Action& action) const {
	if (!checkPreconditionsHold(action)) return false;
	
	if (stateConstraints.size() != 0) { // If we have no constraints, we can spare the cost of creating the new state.
		State s1(_state, computeEffects(action));
		return checkStateConstraintsHold(s1);
	}
	return true;
}

bool StandardApplicabilityManager::checkStateConstraintsHold(const State& s) const {
	for (ScopedConstraint::cptr ctr:stateConstraints) {
		if (!ctr->isSatisfied(s)) return false;
	}
	return true;
}

//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
Fact::vctr StandardApplicabilityManager::computeEffects(const Action& action) const {
	Fact::vctr atoms;
	for (const ScopedEffect::cptr effect:action.getEffects()) {
		atoms.push_back(effect->apply(_state)); // TODO - Note that this won't work for conditional effects where an action might have no effect at all
	}
	return atoms;
}
	

} // namespaces
