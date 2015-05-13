
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <constraints/scoped_constraint.hxx>


namespace fs0 {

class Action;

/**
 * A simple manager that only checks applicability of actions in a non-relaxed setting.
 */
class StandardApplicabilityManager
{
public:
	StandardApplicabilityManager(const State& state, const ScopedConstraint::vcptr& constraints);
		
	StandardApplicabilityManager(const StandardApplicabilityManager& other);
	
	//! Return true iff the preconditions of the applicable entity hold.
	bool checkPreconditionsHold(const Action& action) const;
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const Action& action) const;
	
	bool checkStateConstraintsHold(const State& s) const;
	
	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	Atom::vctr computeEffects(const Action& action) const;
	
protected:
	//! The state
	const State& _state;
	
	//! The state constraints
	const ScopedConstraint::vcptr& stateConstraints;
};

} // namespaces
