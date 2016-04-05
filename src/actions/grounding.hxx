
#pragma once

#include <fs0_types.hxx>

namespace fs0 {

class ProblemInfo;
class ActionSchema;
class GroundAction;
class Binding;

class ActionGrounder {
public:
	//! Grounds the set of given action schemata with all parameter groundings that induce no false preconditions
	//! Returns the new set of grounded actions
	static std::vector<const GroundAction*> ground(const std::vector<const ActionSchema*>& schemata, const ProblemInfo& info);
	
protected:
	//! Helper to ground a schema with a single binding. Returns the expected next action ID, which might be the same
	//! ID that was received, if the grounding was unsuccessful, or a consecutive one, otherwise.
	static unsigned ground(unsigned id, const ActionSchema* schema, const Binding& binding, const ProblemInfo& info, std::vector<const GroundAction*>& grounded);
};

} // namespaces
