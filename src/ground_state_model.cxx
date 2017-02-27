
#include <ground_state_model.hxx>
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

bool GroundStateModel::is_applicable(const State& state, const ActionId& action) const {
	return is_applicable(state, *(_task.getGroundActions()[action]));
}

bool GroundStateModel::is_applicable(const State& state, const ActionType& action) const {
	return _manager->applicable(state, action);
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
	return _manager->applicable(state);
}

ActionManagerI*
GroundStateModel::build_action_manager(const Problem& problem) {
	const auto& actions = problem.getGroundActions();
	const auto& constraints = problem.getStateConstraints();
	const auto& tuple_idx =  problem.get_tuple_index();
    auto strategy = Config::instance().getSuccessorGeneratorType();

    if (strategy == Config::SuccessorGenerationStrategy::naive) {
		LPT_INFO( "cout", "Successor Generator Strategy: Naive");
		return new NaiveActionManager(actions, constraints);
	}

	if ( strategy == Config::SuccessorGenerationStrategy::functional_aware) {
		LPT_INFO( "main", "Successor Generator Strategy: \"Functional Aware\"");
		BasicApplicabilityAnalyzer analyzer(actions, tuple_idx);
		analyzer.build();
		return new SmartActionManager(actions, constraints, tuple_idx, analyzer);
	}else if (strategy == Config::SuccessorGenerationStrategy::match_tree) {
        const StateAtomIndexer& indexer = problem.getStateAtomIndexer();
        if (!indexer.is_fully_binary())
            throw std::runtime_error("Successor Generation Strategy: Match Tree: Variable domains not binary.");
		LPT_INFO( "cout", "Successor Generator Strategy: Match Tree");
		LPT_INFO("cout", "Peak mem. usage before match-tree construction: " << get_peak_memory_in_kb() << " kB.");
		LPT_INFO("cout", "Current mem. usage before match-tree construction: " << get_current_memory_in_kb() << " kB.");

		auto mng = new MatchTreeActionManager(actions, constraints, tuple_idx);
		LPT_INFO("cout", "Match-tree built with " << mng->count() << " nodes.");
		LPT_INFO("cout", "Peak mem. usage after match-tree construction: " << get_peak_memory_in_kb() << " kB.");
		LPT_INFO("cout", "Current mem. usage after match-tree construction: " << get_current_memory_in_kb() << " kB.");
		return mng;
	}

	throw std::runtime_error("Unknown successor generation strategy");
}


} // namespaces
