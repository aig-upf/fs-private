
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>

using namespace fs0::language::fstrips;

namespace fs0 {

class GroundAction; class State;

/**
 * A simple manager that only checks applicability of actions in a non-relaxed setting.
 */
class ApplicabilityManager
{
public:
	ApplicabilityManager(const std::vector<AtomicFormula::cptr>& state_constraints);
		
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const State& state, const GroundAction& action) const;
	
	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	Atom::vctr computeEffects(const State& state, const GroundAction& action) const;
	
	static bool checkFormulaHolds(const std::vector<AtomicFormula::cptr>& formula, const State& state);
	
	//! Checks that all of the given new atoms do not violate domain bounds
	static bool checkAtomsWithinBounds(const std::vector<Atom>& atoms);
	
protected:
	//! The state constraints
	const std::vector<AtomicFormula::cptr>& _state_constraints;
};

} // namespaces

