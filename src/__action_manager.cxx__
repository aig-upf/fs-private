
#include <cassert>
#include <iosfwd>

#include <action_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/projections.hxx>
#include <problem.hxx>
#include <relaxed_action_manager.hxx>

namespace fs0 {


bool ActionManager::checkPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& s0) {
	auto endState = applyPlan(problem, plan, s0);
	// Check first that the plan is valid (pointer not null) and then that leads to a goal state
	return endState && problem.isGoal(*endState);
}

State::ptr ActionManager::applyPlan(const Problem& problem, const ActionPlan& plan, const State& s0) {
	State::ptr s = std::make_shared<State>(s0);
	for (unsigned idx:plan) {
		s = applyAction(problem, problem.getAction(idx), s);
		if (!s) return s;
	}
	return s;
}	

State::ptr ActionManager::applyAction(const Problem& problem, const Action::cptr& action, const State::ptr& s0) {
	StandardApplicabilityManager manager(*s0, problem.getConstraints());
	
	if (!manager.isApplicable(*action)) {
		return State::ptr();
	}
	
	return std::make_shared<State>(*s0, manager.computeEffects(*action)); // Copy everything into the new state and apply the changes
}


//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
bool ActionManager::checkRelaxedPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& seed) {
	RelaxedState layer(seed);
	if (!applyRelaxedPlan(problem, plan, layer)) return false;
	return problem.getConstraintManager()->isGoal(layer);
}

//! Applies the given plan in relaxed mode to the given relaxed state.
// TODO - This might not be precise at all - we need to make clear what we understand by the application of a relaxed plan.
bool ActionManager::applyRelaxedPlan(const Problem& problem, const ActionPlan& plan, RelaxedState& relaxed) {
	RPGData rpg(relaxed);
	for (const auto& idx:plan) {
		const Action& action = *(problem.getAction(idx));
		action.getConstraintManager()->processAction(idx, action, relaxed, rpg);
		RPGData::accumulate(relaxed, rpg);
	}
	return true;
}


} // namespaces

