
#include <fs/core/search/drivers/fully_lifted_driver.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/utils.hxx>

#include <fs/core/actions/grounding.hxx>
#include <fs/core/utils/support.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {
	
FullyLiftedDriver::EnginePT
FullyLiftedDriver::create(const Config& config, LiftedStateModel& model, SearchStats& stats) {
	LPT_INFO("main", "Using the Fully-lifted driver");
	const Problem& problem = model.getTask();
	
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();

	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	auto managers = LiftedActionCSP::create(actions, problem.get_tuple_index(), approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler));
	auto engine = EnginePT(new EngineT(model));
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return engine;
}


LiftedStateModel
FullyLiftedDriver::setup(Problem& problem) const {
	return GroundingSetup::fully_lifted_model(problem);
}

ExitCode 
FullyLiftedDriver::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	LiftedStateModel model = setup(problem);
	SearchStats stats;
	auto engine = create(config, model, stats);
	return Utils::SearchExecution<LiftedStateModel>(model).do_search(*engine, options, start_time, stats);
}

} } // namespaces
