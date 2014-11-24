
#ifndef __APTK_CORE_ACTION_MANAGER_HXX__
#define __APTK_CORE_ACTION_MANAGER_HXX__

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <variable_cache.hxx>
#include <state.hxx>
#include <core_problem.hxx>

namespace aptk { namespace core {
	
/**
 */
class ActionManager
{
public:
	
	/**
	 * Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
	 */
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
		RelaxedActionSetManager manager(*s0);
		JustifiedAction justified(*action);
		
		if (!manager.isApplicable(justified)) {
			return RelaxedState::ptr();
		}
		
		Changeset changeset;
		manager.computeChangeset(justified, changeset);
		return std::make_shared<RelaxedState>(*s0, changeset); // Copy everything into the new state and apply the changeset
	}
	
	static State::ptr applyAction(const CoreAction::cptr& action, const State::ptr& s0) {
		SimpleActionSetManager manager(*s0);
		
		if (!manager.isApplicable(*action)) {
			return State::ptr();
		}
		
		Changeset changeset;
		manager.computeChangeset(*action, changeset);
		return std::make_shared<State>(*s0, changeset); // Copy everything into the new state and apply the changeset
	}	
	
// 	//! Returns true iff the given formula is satisfiable in the (possibly relaxed) given state.
// 	template <class S>
// 	static bool checkFormulaSatisfiable(const PlainConjunctiveFact& formula, const S& state);
// 
// 	//!Returns true iff the given fact is true in the (possibly relaxed) given state.
// 	template <class S>
// 	static bool checkFormulaSatisfiable(const Fact& fact, const S& state);
// 
// 	//! Returns true iff the given negated fact is satisfiable in the (possibly relaxed) given state.
// 	static bool checkFormulaSatisfiable(const NegatedFact& fact, const State& state);
// 	static bool checkFormulaSatisfiable(const NegatedFact& fact, const RelaxedState& state);
};

} } // namespaces

#endif
