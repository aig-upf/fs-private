
#include <fs/core/search/drivers/smart_lifted_driver.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>

#include <fs/core/actions/grounding.hxx>
#include <fs/core/utils/support.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {
	
SmartLiftedDriver::EnginePT
SmartLiftedDriver::create(const Config& config, LiftedStateModel& model, SearchStats& stats) {
	LPT_INFO("main", "Using the Partially-lifted smart driver");
	const Problem& problem = model.getTask();
	
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();

	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	
	// We create smart managers by grounding only wrt the effect heads.
	auto managers = LiftedEffectCSP::create_smart(actions, tuple_index, approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	_heuristic = std::make_unique<HeuristicT>(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);

	
	// If necessary, we constrain the state variables domains and even action/effect CSPs that will be used henceforth
	// by performing a reachability analysis.
	if (config.getOption<bool>("reachability_analysis")) {
		LPT_INFO("main", "Applying reachability analysis");
		RPGIndex graph = _heuristic->compute_full_graph(problem.getInitialState());
		LiftedEffectCSP::prune_unreachable(_heuristic->get_managers(), graph);
	}
	
	auto engine = EnginePT(new EngineT(model));
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers, config.getOption<bool>("verbose_stats", false));
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return engine;
}


ExitCode 
SmartLiftedDriver::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	LiftedStateModel model = GroundingSetup::fully_lifted_model(problem);
	SearchStats stats;
	auto engine = create(config, model, stats);
	return Utils::SearchExecution<LiftedStateModel>(model).do_search(*engine, options, start_time, stats);
}

} } // namespaces
