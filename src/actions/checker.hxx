
#pragma once

#include <memory>
#include <vector>

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
	static State::ptr applyPlan(const Problem& problem, const ActionPlan& plan, const State& s0);
	
	static State::ptr applyAction(const Problem& problem, const GroundAction& action, const State::ptr& s0);
};


} // namespaces
