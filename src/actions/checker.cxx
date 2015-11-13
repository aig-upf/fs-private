
#include <actions/checker.hxx>
#include <problem.hxx>
#include <applicability/applicability_manager.hxx>

namespace fs0 {

bool Checker::checkPlanSuccessful(const Problem& problem, const ActionPlan& plan, const State& s0) {
	auto s1 = applyPlan(problem, plan, s0);
	// Check first that the plan is valid (pointer not null) and then that leads to a goal state
	return s1 && ApplicabilityManager::checkFormulaHolds(problem.getGoalConditions(), *s1);
}

std::shared_ptr<State> Checker::applyPlan(const Problem& problem, const ActionPlan& plan, const State& s0) {
	std::shared_ptr<State> s = std::make_shared<State>(s0);
	const auto& actions = problem.getGroundActions();
	for (unsigned idx:plan) {
		s = applyAction(problem, *actions[idx], s);
		if (!s) return s;
	}
	return s;
}	
std::shared_ptr<State> Checker::applyAction(const Problem& problem, const GroundAction& action, const std::shared_ptr<State> s0) {
	ApplicabilityManager manager(problem.getStateConstraints());
	
	if (!manager.isApplicable(*s0, action)) {
		return nullptr;
	}
	
	return std::make_shared<State>(*s0, manager.computeEffects(*s0, action)); // Copy everything into the new state and apply the changes
}


} // namespaces
