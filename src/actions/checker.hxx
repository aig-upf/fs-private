
#pragma once

#include <fs0_types.hxx>
#include <actions/ground_action.hxx>
#include <state.hxx>

namespace fs0 {

class State; class Problem;

class Checker {
public:
	//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
	static bool checkPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& s0);
	
protected:
	static std::shared_ptr<State> applyPlan(const Problem& problem, const ActionPlan& plan, const State& s0);
	
	static std::shared_ptr<State> applyAction(const Problem& problem, const GroundAction& action, const std::shared_ptr<State> s0);
};


} // namespaces
