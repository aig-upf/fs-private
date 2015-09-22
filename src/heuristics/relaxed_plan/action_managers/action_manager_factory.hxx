
#pragma once

#include <actions/ground_action.hxx>
#include <heuristics/relaxed_plan/action_managers/base_action_manager.hxx>

namespace fs0 {

class Problem; class GroundAction;

/**
 * An applicability manager operating on a delete-free relaxation of the problem in order to
 * assess the applicability of actions (disregarding state constraints is part of the relaxation).
 *
 * There are two possible ways of checking the applicability of an action:
 * (1) In the simplest case, when all of the action preconditions and effects are at most unary
 *     (effects can be 0-ary, but note that 0-ary preconditions are static and should be detected at compile-time),
 *     we use an exact method implemented by the DirectActionManager.
 *
 * (2) In the general case, i.e. when the above restrictions on the arity of the procedures do not hold,
 *     we use a GecodeActionManager that involves a Gecode model.
 */
class ActionManagerFactory
{
public:
	//! A factory method to create the appropriate manager.
	static BaseActionManager* create(const GroundAction& action);
	
	//! A helper to instantiate a suitable base action manager for a set of actions
	static std::vector<std::shared_ptr<BaseActionManager>> create(const std::vector<GroundAction::cptr>& actions);

protected:
	//! Returns true iff at the properties of the action make necessary to use the gecode action manager.
	static bool gecodeManagerNeeded(const GroundAction& action);
};

} // namespaces