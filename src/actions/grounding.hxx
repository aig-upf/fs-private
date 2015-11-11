
#pragma once

#include <fs0_types.hxx>
#include <actions/ground_action.hxx>
#include <actions/action_schema.hxx>

namespace fs0 {

class ActionGrounder {
public:
	//! Grounds the set of given action schemata with all parameter groundings that induce no false preconditions
	//! Returns the new set of grounded actions
	static std::vector<GroundAction::cptr> ground(const std::vector<ActionSchema::cptr>& schemata, const ProblemInfo& info);
	
protected:
	//! Helper to ground a schema with a single binding
	static void ground(fs0::ActionSchema::cptr schema, const Binding& binding, const fs0::ProblemInfo& info, std::vector< fs0::GroundAction::cptr >& grounded);
};

} // namespaces
