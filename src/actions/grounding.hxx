
#pragma once

#include <string>
#include <fs_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Term; class ActionEffect; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ProblemInfo;
class ActionData;
class ActionBase;
class GroundAction;
class Binding;
class PartiallyGroundedAction;

//! This exception is thrown whenever a variable cannot be resolved
class TooManyGroundActionsError : public std::runtime_error {
public:
	TooManyGroundActionsError( unsigned long num_actions ) : std::runtime_error("The number of ground actions is too high: " + std::to_string(num_actions)) {}
};

class ActionGrounder {
public:
	static const unsigned long MAX_GROUND_ACTIONS = 100000000;
	
	//! Grounds the set of given action schemata with all parameter groundings that induce no false preconditions
	//! Returns the new set of grounded actions
// 	static std::vector<const ActionBase*> ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info);
	
	//! Process the given action data to consolidate state variables, etc.
	static ActionData* process_action_data(const ActionData& action_data, const ProblemInfo& info);
	
	//! Binds a partially grounded action with a full binding, disregarding the ID of the resulting grounded action
	static GroundAction* bind(const PartiallyGroundedAction& action, const Binding& binding, const ProblemInfo& info);
	
	//!
	static std::vector<const PartiallyGroundedAction*> compile_action_parameters_away(const PartiallyGroundedAction* schema, unsigned effect_idx, const ProblemInfo& info);

	//! Generate fully-lifted actions from the action schema data
	static std::vector<const PartiallyGroundedAction*> fully_lifted(const std::vector<const ActionData*>& action_data, const ProblemInfo& info);
	
	static std::vector<const GroundAction*> fully_ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info);
	
	static const std::vector<const fs::ActionEffect*> compile_nested_fluents_away(const fs::ActionEffect* effect, const ProblemInfo& info);
	
	//! Helper to ground a schema with a single binding. Returns the expected next action ID, which might be the same
	//! ID that was received, if the grounding was unsuccessful, or a consecutive one, otherwise.
	static unsigned ground(unsigned id, const ActionData* data, const Binding& binding, const ProblemInfo& info, std::vector<const GroundAction*>& grounded);
	
protected:
	//! Process the action schema with a given parameter binding and return the corresponding GroundAction
	//! A nullptr is returned if the action is detected to be statically non-applicable
	static GroundAction* full_binding(unsigned id, const ActionData& action_data, const Binding& binding, const ProblemInfo& info);
	static PartiallyGroundedAction* partial_binding(const ActionData& action_data, const Binding& binding, const ProblemInfo& info);
	
	//! Return the non-constant terms that are present as first-level subterms of the head, i.e.
	//! for an effect clear(loc(b), g(f(c))) := true, this would return _only_ loc(b) and g(f(c)), but not f(c).
// 	static std::vector<const fs::Term*> collect_effect_non_constant_subterms(const fs::ActionEffect* effect);
};

} // namespaces
