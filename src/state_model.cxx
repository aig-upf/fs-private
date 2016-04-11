
#include <state_model.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>
#include <actions/applicable_action_set.hxx>
#include <applicability/applicability_manager.hxx>
#include <problem_info.hxx>

namespace fs0 {

State FS0StateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// Ugly, but this way we make it fit the search engine interface without further changes,
	// and this is only called once per search.
	return State(task.getInitialState());
}

bool FS0StateModel::goal(const State& state) const {
	return task.getGoalSatManager().satisfied(state);
}

State FS0StateModel::next(const State& state, const GroundAction::IdType& actionIdx) const {
	return next(state, *(task.getGroundActions()[actionIdx]));
} 

State FS0StateModel::next(const State& state, const GroundAction& a) const { 
	ApplicabilityManager manager(task.getStateConstraints());
	return State(state, manager.computeEffects(state, a)); // Copy everything into the new state and apply the changeset
}

void FS0StateModel::print(std::ostream& os) const {
	os << task;
}

GroundAction::ApplicableSet FS0StateModel::applicable_actions(const State& state) const {
	return ApplicableActionSet(ApplicabilityManager(task.getStateConstraints()), state, task.getGroundActions());
}

} // namespaces

