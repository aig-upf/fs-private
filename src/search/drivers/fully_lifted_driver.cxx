
#include <search/drivers/fully_lifted_driver.hxx>
#include <search/drivers/setups.hxx>
#include <search/utils.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <constraints/gecode/handlers/lifted_action_csp.hxx>
#include <constraints/gecode/handlers/formula_csp.hxx>

#include <state.hxx>
#include <actions/lifted_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <utils/support.hxx>

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
	auto engine = EnginePT(new EngineT(model, *_heuristic));
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return engine;
}


LiftedStateModel
FullyLiftedDriver::setup(Problem& problem) const {
	return GroundingSetup::fully_lifted_model(problem);
}

void 
FullyLiftedDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	LiftedStateModel model = setup(problem);
	SearchStats stats;
	auto engine = create(config, model, stats);
	Utils::do_search(*engine, model, out_dir, start_time, stats);
}

} } // namespaces
