
#pragma once

#include <fs0_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class GroundAction; class State; class Atom;

//! A simple manager that only checks applicability of actions in a non-relaxed setting.
class ApplicabilityManager {
public:
	ApplicabilityManager(const fs::Formula* state_constraints);
		
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const State& state, const GroundAction& action) const;
	
	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	std::vector<Atom> computeEffects(const State& state, const GroundAction& action) const;
	
	static bool checkFormulaHolds(const fs::Formula* formula, const State& state);
	
	//! Checks that all of the given new atoms do not violate domain bounds
	static bool checkAtomsWithinBounds(const std::vector<Atom>& atoms);
	
protected:
	//! The state constraints
	const fs::Formula* _state_constraints;
};

} // namespaces

