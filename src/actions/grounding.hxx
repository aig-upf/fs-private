
#pragma once

#include <fs_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class StateVariable; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ProblemInfo;
class ActionData;
class ActionBase;
class GroundAction;
class Binding;
class PartiallyGroundedAction;

class ActionGrounder {
public:
	//! Grounds the set of given action schemata with all parameter groundings that induce no false preconditions
	//! Returns the new set of grounded actions
// 	static std::vector<const ActionBase*> ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info);
	
	//! Process the given action data to consolidate state variables, etc.
	static ActionData* process_action_data(const ActionData& action_data, const ProblemInfo& info);
	
	//! Binds a partially grounded action with a full binding, disregarding the ID of the resulting grounded action
	static GroundAction* bind(const PartiallyGroundedAction& action, const Binding& binding, const ProblemInfo& info);
	
	//!
	static std::vector<const PartiallyGroundedAction*> flatten_effect_head(const PartiallyGroundedAction* schema, unsigned effect_idx, const ProblemInfo& info);

	//! Generate fully-lifted actions from the action schema data
	static std::vector<const PartiallyGroundedAction*> fully_lifted(const std::vector<const ActionData*>& action_data, const ProblemInfo& info);
	
	static std::vector<const GroundAction*> fully_ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info);
	
	
protected:
	//! Helper to ground a schema with a single binding. Returns the expected next action ID, which might be the same
	//! ID that was received, if the grounding was unsuccessful, or a consecutive one, otherwise.
	static unsigned ground(unsigned id, const ActionData* data, const Binding& binding, const ProblemInfo& info, std::vector<const GroundAction*>& grounded);
	
	//! Process the action schema with a given parameter binding and return the corresponding GroundAction
	//! A nullptr is returned if the action is detected to be statically non-applicable
	static GroundAction* full_binding(unsigned id, const ActionData& action_data, const Binding& binding, const ProblemInfo& info);
	static PartiallyGroundedAction* partial_binding(const ActionData& action_data, const Binding& binding, const ProblemInfo& info);
	
	
	static std::vector<const fs::StateVariable*> collect_effect_head_variables(const PartiallyGroundedAction* action);
	
	static std::vector<const PartiallyGroundedAction*> compile_nested_fluents_away(const std::vector<const PartiallyGroundedAction*>& actions, const ProblemInfo& info);
};

} // namespaces
