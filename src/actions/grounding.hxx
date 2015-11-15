
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
	//! Helper to ground a schema with a single binding
	static void ground(const ActionSchema* schema, const Binding& binding, const ProblemInfo& info, std::vector<GroundAction*>& grounded);
};

} // namespaces
