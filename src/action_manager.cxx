
#include <cassert>
#include <iosfwd>

#include <action_manager.hxx>
#include <heuristics/changeset.hxx>
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
	
	return std::make_shared<State>(*s0, manager.computeEffects(*action)); // Copy everything into the new state and apply the changeset
}


//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
bool ActionManager::checkRelaxedPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& seed) {
	RelaxedState relaxed(seed);
	if (!applyRelaxedPlan(problem, plan, seed, relaxed)) return false;
	return problem.getConstraintManager()->isGoal(relaxed);
}

//! Applies the given plan in relaxed mode to the given relaxed state.
bool ActionManager::applyRelaxedPlan(const Problem& problem, const ActionPlan& plan, const State& seed, RelaxedState& relaxed) {
	for (const auto& idx:plan) {
		if (!applyRelaxedAction(*(problem.getAction(idx)), seed, relaxed)) return false;
	}
	return true;
}

bool ActionManager::applyRelaxedAction(const Action& action, const State& seed, RelaxedState& s) {
	DomainMap projection = Projections::projectToActionVariables(s, action);

	Fact::vctr causes;
	if (action.getConstraintManager()->checkPreconditionApplicability(action, seed, projection, causes)) { // The action is applicable - we ignore the causes
		Changeset changeset(seed, s);
		action.getConstraintManager()->computeChangeset(action, projection, changeset);
		s.accumulate(changeset);
		return true;
	}
	return false;
}


} // namespaces

