
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>
#include <utils/projections.hxx>
#include <app_entity.hxx>


namespace fs0 {


/**
 * A simple manager that only checks applicability for non-relaxed states.
 * This means that we can get rid of causal data structures, etc.
 */
class SimpleActionSetManager
{
public:
	SimpleActionSetManager(const State& state, const ScopedConstraint::vcptr& constraints)
		: _state(state), _constraints(constraints) {}
		
	SimpleActionSetManager(const SimpleActionSetManager& other)
		: _state(other._state), _constraints(other._constraints) {}
	
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
		
		if (_constraints.size() != 0) { // If we have no constraints, we can spare the cost of creating the new state.
			FactSet atoms;
			computeChangeset(action, atoms);
			State s1(_state, atoms);
			return checkStateConstraintsHold(s1);
		}
		return true;
	}
	
	bool checkStateConstraintsHold(const State& s) const {
		for (ScopedConstraint::cptr ctr:_constraints) {
			if (!ctr->isSatisfied(s)) return false;
		}
		return true;
	}
	
	void computeChangeset(const Action& action, FactSet& atoms) const {
		for (const ScopedEffect::cptr effect:action.getEffects()) {
			atoms.insert(effect->apply(_state)); // TODO - Note that this won't work for conditional effects where an action might have no effect at all
		}
	}
	
protected:
	//! The state
	const State& _state;
	
	//! The state constraints
	const ScopedConstraint::vcptr& _constraints;
};

} // namespaces
