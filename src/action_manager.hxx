
#pragma once

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <state.hxx>
#include <core_problem.hxx>

namespace aptk { namespace core {
	
/**
 * Some helper methods.
 * TODO - This should all be refactored into the existing Relaxed and NonRelaxed managers.
 */
class ActionManager
{
public:
	
	//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
	template <typename StateType>
	static bool checkPlanSuccessful(const Problem& problem, const ActionPlan& plan, const StateType& s0) {
		auto endState = applyPlan(problem, plan, s0);
		// Check first that the plan is valid (pointer not null) and then that leads to a goal state
		return endState && problem.isGoal(*endState);
	}
	
	/**
	 * Applies a plan to a given state and returns the produced state.
	 * Returns a null State:ptr if the plan is incorrect, i.e. contains some inapplicable action.
	 */
	template <typename StateType>
	static typename StateType::ptr applyPlan(const Problem& problem, const ActionPlan& plan, const StateType& s0) {
		typename StateType::ptr s = std::make_shared<StateType>(s0);
		for (const auto& idx:plan) {
			s = applyAction(problem.getAction(idx), s);
			if (!s) return s;
		}
		return s;
	}
	
	/**
	 * Applies a single action to a given state and returns the produced state.
	 * Returns a null State:ptr if the action is not applicable.
	 */
	static RelaxedState::ptr applyAction(const CoreAction::cptr& action, const RelaxedState::ptr& s0) {
		RelaxedActionSetManager manager(Problem::getCurrentProblem()->getConstraints());
		DomainSet projection = manager.projectValues(*s0, *action);
		auto res = manager.isApplicable(*action, projection);
		
		if (!res.first) {
			return RelaxedState::ptr();
		}
		
		Changeset changeset;
		manager.computeChangeset(*action, projection, changeset);
		return std::make_shared<RelaxedState>(*s0, changeset); // Copy everything into the new state and apply the changeset
	}
	
	static State::ptr applyAction(const CoreAction::cptr& action, const State::ptr& s0) {
		SimpleActionSetManager manager(*s0, Problem::getCurrentProblem()->getConstraints());
		
		if (!manager.isApplicable(*action)) {
			return State::ptr();
		}
		
		Changeset changeset;
		manager.computeChangeset(*action, changeset);
		return std::make_shared<State>(*s0, changeset); // Copy everything into the new state and apply the changeset
	}

};

} } // namespaces
