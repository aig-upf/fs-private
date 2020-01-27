
#include <fs/core/search/drivers/fully_lifted_driver.hxx>
#include <memory>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/utils.hxx>

#include <fs/core/actions/grounding.hxx>
#include <fs/core/utils/support.hxx>

using namespace fs0::gecode;

namespace fs0::drivers {
	
FullyLiftedDriver::EnginePT
FullyLiftedDriver::create(const Config& config, CSPLiftedStateModel& model, SearchStats& stats) {
	LPT_INFO("main", "Using the Fully-lifted driver");
	const Problem& problem = model.getTask();
	
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();

	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	auto managers = LiftedActionCSP::create(actions, problem.get_tuple_index(), approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	_heuristic = std::make_unique<HeuristicT>(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	auto engine = std::make_unique<EngineT>(model);
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers, config.getOption<bool>("verbose_stats", false));
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return engine;
}


CSPLiftedStateModel
FullyLiftedDriver::setup(Problem& problem) const {
	return GroundingSetup::csp_lifted_model(problem);
}

ExitCode 
FullyLiftedDriver::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	CSPLiftedStateModel model = setup(problem);
	SearchStats stats;
	auto engine = create(config, model, stats);
	return Utils::SearchExecution<CSPLiftedStateModel>(model).do_search(*engine, options, start_time, stats);
}

} // namespaces
