
#include <models/ground_state_model.hxx>
#include "simple_state_model.hxx"
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>
#include <utils/config.hxx>
#include <applicability/match_tree.hxx>
#include <lapkt/tools/logging.hxx>
#include <utils/system.hxx>

namespace fs0 {

GroundStateModel::GroundStateModel(const Problem& problem) :
	_task(problem),
	_manager(build_action_manager(problem))
{}

State GroundStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// This is only called once per search.
	return State(_task.getInitialState());
}

bool GroundStateModel::goal(const State& state) const {
	return _task.getGoalSatManager().satisfied(state);
}

bool GroundStateModel::is_applicable(const State& state, const ActionId& action, bool enforce_state_constraints) const {
	return is_applicable(state, *(_task.getGroundActions()[action]), enforce_state_constraints);
}

bool GroundStateModel::is_applicable(const State& state, const ActionType& action, bool enforce_state_constraints) const {
	return _manager->applicable(state, action, enforce_state_constraints);
}

State GroundStateModel::next(const State& state, const GroundAction::IdType& actionIdx) const {
	return next(state, *(_task.getGroundActions()[actionIdx]));
}

State GroundStateModel::next(const State& state, const GroundAction& a) const {
	NaiveApplicabilityManager::computeEffects(state, a, _effects_cache);
	return State(state, _effects_cache); // Copy everything into the new state and apply the changeset
}

GroundApplicableSet GroundStateModel::applicable_actions(const State& state, bool enforce_state_constraints) const {
	return _manager->applicable(state, enforce_state_constraints);
}

ActionManagerI*
GroundStateModel::build_action_manager(const Problem& problem) {
	return SimpleStateModel::build_action_manager(problem);
}


} // namespaces
