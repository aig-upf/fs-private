
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <constraints/scoped_constraint.hxx>
#include <utils/projections.hxx>
#include <actions.hxx>


namespace fs0 {


/**
 * A simple manager that only checks applicability of actions in a non-relaxed setting.
 */
class StandardApplicabilityManager
{
public:
	StandardApplicabilityManager(const State& state, const ScopedConstraint::vcptr& constraints)
		: _state(state), stateConstraints(constraints) {}
		
	StandardApplicabilityManager(const StandardApplicabilityManager& other)
		: _state(other._state), stateConstraints(other.stateConstraints) {}
	
	//! Return true iff the preconditions of the applicable entity hold.
	bool checkPreconditionsHold(const Action& action) const {
		for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
			if (!constraint->isSatisfied(_state))
				return false;
		}
		return true;
	}
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const Action& action) const {
		if (!checkPreconditionsHold(action)) return false;
		
		if (stateConstraints.size() != 0) { // If we have no constraints, we can spare the cost of creating the new state.
			State s1(_state, computeEffects(action));
			return checkStateConstraintsHold(s1);
		}
		return true;
	}
	
	bool checkStateConstraintsHold(const State& s) const {
		for (ScopedConstraint::cptr ctr:stateConstraints) {
			if (!ctr->isSatisfied(s)) return false;
		}
		return true;
	}
	
	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	Fact::vctr computeEffects(const Action& action) const {
		Fact::vctr atoms;
		for (const ScopedEffect::cptr effect:action.getEffects()) {
			atoms.push_back(effect->apply(_state)); // TODO - Note that this won't work for conditional effects where an action might have no effect at all
		}
		return atoms;
	}
	
protected:
	//! The state
	const State& _state;
	
	//! The state constraints
	const ScopedConstraint::vcptr& stateConstraints;
};

} // namespaces
