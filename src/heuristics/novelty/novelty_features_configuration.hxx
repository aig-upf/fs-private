
#pragma once

#include <utils/config.hxx>

namespace fs0 {

class NoveltyFeaturesConfiguration {
public:
	enum class Type {ALL, GOAL_ONLY, RELEVANT1};
	
	
	//! Create a NoveltyFeaturesConfiguration object from a global configuration object
	NoveltyFeaturesConfiguration(const Config& config) :
		_use_state_vars(config.getOption<bool>("width.use_state_vars")),
		_use_goal(config.getOption<bool>("width.use_goal")),
		_use_actions(config.getOption<bool>("width.use_actions")),
		_type(parse_type(config.getOption<std::string>("width.features")))
	{}

	bool useStateVars() const { return _use_state_vars; }
	bool useGoal() const { return _use_goal; }
	bool useActions() const { return _use_actions; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const NoveltyFeaturesConfiguration&  o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "Novelty Feature Configuration[";
		os << "state variables: " << ( _use_state_vars ? "yes" : "no");
		os << ", goal: " << (_use_goal ? "yes" : "no");
		os << ", actions: " << (_use_actions ? "yes" : "no");
		os << "]";
		return os;
	}
	
	Type getType() const { return _type; }
	
	
protected:
	bool _use_state_vars;
	bool _use_goal;
	bool _use_actions;
	
	Type _type;
	
	Type parse_type(const std::string& type) {
		if (type == "all") return Type::ALL;
		if (type == "goal") return Type::GOAL_ONLY;
		if (type == "relevant1") return Type::RELEVANT1;
		throw std::runtime_error("Unknown feature selection type " + std::string(type));
	}
};

}
