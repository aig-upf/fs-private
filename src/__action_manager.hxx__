
#pragma once

#include <actions.hxx>
#include <fs0_types.hxx>
#include <state.hxx>

namespace fs0 {

class Problem;

/**
 * Some helper methods.
 * TODO - This should all be refactored into the existing Relaxed and NonRelaxed managers.
 */
class ActionManager
{
public:
	
	//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
	static bool checkPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& s0);
	
	static State::ptr applyPlan(const Problem& problem, const ActionPlan& plan, const State& s0);
	
	static State::ptr applyAction(const Problem& problem, const Action::cptr& action, const State::ptr& s0);


	//! Returns true iff the given plan is valid and leads to a goal state, when applied to state s0 in relaxed mode.
	static bool checkRelaxedPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& seed);

	//! Applies the given plan in relaxed mode to the given relaxed state.
	static bool applyRelaxedPlan(const Problem& problem, const ActionPlan& plan, RelaxedState& relaxed);
};

} // namespaces
