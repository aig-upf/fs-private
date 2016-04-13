
#include <ground_state_model.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>
#include <actions/applicable_action_set.hxx>

namespace fs0 {

State GroundStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// Ugly, but this way we make it fit the search engine interface without further changes,
	// and this is only called once per search.
	return State(task.getInitialState());
}

bool GroundStateModel::goal(const State& state) const {
	return task.getGoalSatManager().satisfied(state);
}

State GroundStateModel::next(const State& state, const GroundAction::IdType& actionIdx) const {
	return next(state, *(task.getGroundActions()[actionIdx]));
} 

State GroundStateModel::next(const State& state, const GroundAction& a) const { 
	ApplicabilityManager manager(task.getStateConstraints());
	return State(state, manager.computeEffects(state, a)); // Copy everything into the new state and apply the changeset
}

void GroundStateModel::print(std::ostream& os) const {
	os << task;
}

GroundAction::ApplicableSet GroundStateModel::applicable_actions(const State& state) const {
	return ApplicableActionSet(ApplicabilityManager(task.getStateConstraints()), state, task.getGroundActions());
}

} // namespaces

