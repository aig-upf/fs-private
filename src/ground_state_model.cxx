
#include <ground_state_model.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>
#include "problem_info.hxx"

namespace fs0 {
	
void 
GroundStateModel::nullify_trajectory(State& state) const {
	if (_remove_trajectory) {
		state.accumulate(_null_trajectory); // HACK HACK HACK Nullify the "traj(rob)" state variable
	}
}

GroundStateModel::GroundStateModel(const Problem& problem, BasicApplicabilityAnalyzer* analyzer, bool remove_trajectory) :
	_task(problem),
	_manager(build_action_manager(problem, analyzer)),
	_null_trajectory_atom(fs0::ProblemInfo::getInstance().getVariableId("traj(rob)"), fs0::ProblemInfo::getInstance().getObjectId("t0")),
	_null_trajectory(),
	_remove_trajectory(false)
{
	_null_trajectory.push_back(_null_trajectory_atom);
}

State GroundStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// This is only called once per search.
	State init(_task.getInitialState());
	nullify_trajectory(init);
	return init;
}

bool GroundStateModel::goal(const State& state) const {
	return _task.getGoalSatManager().satisfied(state);
}

bool GroundStateModel::is_applicable(const State& state, const ActionId& action) const {
	return is_applicable(state, *(_task.getGroundActions()[action]));
}

bool GroundStateModel::is_applicable(const State& state, const ActionType& action) const {
	return _manager.applicable(state, action);
}

State GroundStateModel::next(const State& state, const GroundAction::IdType& actionIdx) const {
	return next(state, *(_task.getGroundActions()[actionIdx]));
} 

State GroundStateModel::next(const State& state, const GroundAction& a) const {
	NaiveApplicabilityManager::computeEffects(state, a, _effects_cache);
	if (_remove_trajectory) {
		_effects_cache.push_back(_null_trajectory_atom);
	}
	return State(state, _effects_cache); // Copy everything into the new state and apply the changeset
}

void GroundStateModel::print(std::ostream& os) const { os << _task; }

GroundApplicableSet GroundStateModel::applicable_actions(const State& state) const {
	return _manager.applicable(state);
}

SmartActionManager
GroundStateModel::build_action_manager(const Problem& problem, BasicApplicabilityAnalyzer* analyzer) {
	const auto& actions = problem.getGroundActions();
	const auto& constraints = problem.getStateConstraints();
	const auto& tuple_idx =  problem.get_tuple_index();
	if (analyzer == nullptr) analyzer = new BasicApplicabilityAnalyzer(actions, tuple_idx);
	return SmartActionManager(actions, constraints, tuple_idx, analyzer);
}


} // namespaces

