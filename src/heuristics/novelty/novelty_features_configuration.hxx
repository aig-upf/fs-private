
#pragma once

#include <utils/config.hxx>

namespace fs0 {

class NoveltyFeaturesConfiguration {
public:
	NoveltyFeaturesConfiguration(bool use_state_vars, bool use_goal, bool use_actions)
		: _use_state_vars(use_state_vars), _use_goal(use_goal), _use_actions(use_actions) {}
	
	//! Create a NoveltyFeaturesConfiguration object from a global configuration object
	NoveltyFeaturesConfiguration(const Config& config)
		: NoveltyFeaturesConfiguration(
			config.getOption<bool>("width.use_state_vars"),
			config.getOption<bool>("width.use_goal"),
			config.getOption<bool>("width.use_actions"))
	{}

	bool useStateVars() const { return _use_state_vars; }
	bool useGoal() const { return _use_goal; }
	bool useActions() const { return _use_actions; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const NoveltyFeaturesConfiguration&  o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "Novelty Feature Configuration[";
		os << "state variables: " << ( _use_state_vars ? "yes" : "no");
		os << "goal: " << (_use_goal ? "yes" : "no");
		os << "actions: " << (_use_actions ? "yes" : "no");
		os << "]";
		return os;
	}
	
protected:
	bool _use_state_vars;
	bool _use_goal;
	bool _use_actions;
};

}
