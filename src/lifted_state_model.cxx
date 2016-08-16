
#include <lifted_state_model.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/lifted_action_iterator.hxx>
#include <actions/actions.hxx>


namespace fs0 {

State LiftedStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// Ugly, but this way we make it fit the search engine interface without further changes,
	// and this is only called once per search.
	return State(task.getInitialState());
}

bool LiftedStateModel::goal(const State& state) const {
	return task.getGoalSatManager().satisfied(state);
}

bool LiftedStateModel::is_applicable(const State& state, const ActionType& action) const {
	auto ground_action = action.generate();
	bool res = is_applicable(state, *ground_action);
	delete ground_action;
	return res;
}

bool LiftedStateModel::is_applicable(const State& state, const GroundAction& action) const {
	ApplicabilityManager manager(task.getStateConstraints());
	return manager.isApplicable(state, action);
}

State LiftedStateModel::next(const State& state, const LiftedActionID& action) const {
	auto ground_action = action.generate();
	auto s1 = next(state, *ground_action);
	delete ground_action;
	return s1;
}

State LiftedStateModel::next(const State& state, const GroundAction& action) const { 
	ApplicabilityManager manager(task.getStateConstraints());
	assert(manager.isApplicable(state, action));
	return State(state, manager.computeEffects(state, action)); // Copy everything into the new state and apply the changeset
}


void LiftedStateModel::print(std::ostream& os) const {
	os << task;
}

gecode::LiftedActionIterator LiftedStateModel::applicable_actions(const State& state) const {
	return gecode::LiftedActionIterator(state, _handlers, task.getStateConstraints());
}

} // namespaces

