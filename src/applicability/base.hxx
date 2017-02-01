
#pragma once


namespace fs0 {

class State;
class GroundAction;
class GroundApplicableSet;

class ActionManagerI {
public:
	using ApplicableSet = GroundApplicableSet;

	virtual ~ActionManagerI() = default;
	
	virtual const std::vector<const GroundAction*>& getAllActions() const = 0;

	//! Return the set of all actions applicable in a given state
	virtual ApplicableSet applicable(const State& state) const = 0;

	//! Return whether the given action is applicable in the given state
	virtual bool applicable(const State& state, const GroundAction& action) const = 0;
};

} // namespaces
