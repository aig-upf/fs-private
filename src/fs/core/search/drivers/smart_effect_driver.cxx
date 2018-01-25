
#include <fs/core/search/drivers/smart_effect_driver.hxx>

#include <fs/core/problem_info.hxx>
#include <fs/core/search/utils.hxx>

#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/utils/support.hxx>
#include <fs/core/constraints/gecode/handlers/ground_effect_csp.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace drivers {

gecode::SmartRPG*
SmartEffectDriver::configure_heuristic(const Problem& problem, const Config& config) {
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();

	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	auto managers = LiftedEffectCSP::create_smart(actions, tuple_index, approximate, novelty);
//	const auto& actions = problem.getGroundActions();
//	auto managers = GroundEffectCSP::create(actions, tuple_index, approximate, novelty);

    if ( config.getOption<bool>("use_state_constraints_in_heuristic_goal_evaluation") ) {
	    const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	    ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	    auto heuristic = new SmartRPG(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);

	    return heuristic;
    }

    const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), {});
    ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
    auto heuristic = new SmartRPG(problem, problem.getGoalConditions(), {}, std::move(managers), extension_handler);


    return heuristic;
}

SmartEffectDriver::EnginePT
SmartEffectDriver::create(const Config& config, const GroundStateModel& model, SearchStats& stats) {
	LPT_INFO("main", "Using the smart-effect driver");
	const Problem& problem = model.getTask();
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();

	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();

	_heuristic = std::unique_ptr<SmartRPG>(configure_heuristic(problem, config));

	// If necessary, we constrain the state variables domains and even action/effect CSPs that will be used henceforth
	// by performing a reachability analysis.
	if (config.getOption("reachability_analysis")) {
		LPT_INFO("main", "Applying reachability analysis");
		RPGIndex graph = _heuristic->compute_full_graph(problem.getInitialState());
		LiftedEffectCSP::prune_unreachable(_heuristic->get_managers(), graph);
	}

	EHCSearch<SmartRPG>* ehc = nullptr;
	if (config.getOption("ehc")) {
		// TODO Apply reachability analysis for the EHC heuristic as well
		auto ehc_managers = LiftedEffectCSP::create_smart(actions,  problem.get_tuple_index(), approximate, novelty);
		const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
		ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
		SmartRPG ehc_heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(ehc_managers), extension_handler);
		ehc = new EHCSearch<SmartRPG>(model, std::move(ehc_heuristic), config.getOption("helpful_actions"), stats);
	}

	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, SmartRPG>(config, *_heuristic, stats, _handlers);
	if (config.requiresHelpfulnessAssessment()) {
		EventUtils::setup_HA_observer<NodeT>(_handlers);
	}

	auto engine = new GBFST(model);
	lapkt::events::subscribe(*engine, _handlers);

	return EnginePT(new EngineT(problem, engine, ehc));
}

GroundStateModel
SmartEffectDriver::setup(Problem& problem) {
	return GroundingSetup::ground_search_lifted_heuristic(problem);
}


ExitCode
SmartEffectDriver::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	GroundStateModel model = setup(problem);
	SearchStats stats;
	bool actionless = model.getTask().getPartiallyGroundedActions().empty();
	auto engine = create(config, model, stats);
	return Utils::SearchExecution<GroundStateModel>(model).do_search(*engine, options, start_time, stats, actionless);

}

} } // namespaces
