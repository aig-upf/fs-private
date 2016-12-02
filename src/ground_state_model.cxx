
#include <ground_state_model.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>

namespace fs0 {

GroundStateModel::GroundStateModel(const Problem& problem, BasicApplicabilityAnalyzer* analyzer) :
	_task(problem),
	_manager(build_action_manager(problem, analyzer))
{}

State GroundStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// This is only called once per search.
	return State(_task.getInitialState());
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
	if (analyzer == nullptr) {
		analyzer = new BasicApplicabilityAnalyzer(actions, tuple_idx);
		analyzer->build();
	}
	return SmartActionManager(actions, constraints, tuple_idx, analyzer);
}


} // namespaces

