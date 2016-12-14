
#include <search/drivers/smart_lifted_driver.hxx>
#include "setups.hxx"
#include <search/utils.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <constraints/gecode/handlers/lifted_action_csp.hxx>
#include <constraints/gecode/handlers/formula_csp.hxx>
#include <constraints/gecode/handlers/lifted_effect_csp.hxx>

#include <state.hxx>
#include <actions/lifted_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <utils/support.hxx>

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
	
	_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler));

	
	// If necessary, we constrain the state variables domains and even action/effect CSPs that will be used henceforth
	// by performing a reachability analysis.
	if (config.getOption<bool>("reachability_analysis")) {
		LPT_INFO("main", "Applying reachability analysis");
		RPGIndex graph = _heuristic->compute_full_graph(problem.getInitialState());
		LiftedEffectCSP::prune_unreachable(_heuristic->get_managers(), graph);
	}
	
	auto engine = EnginePT(new EngineT(model, *_heuristic));
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return engine;
}


LiftedStateModel
SmartLiftedDriver::setup(Problem& problem) const {
	// We set up a lifted model with the action schemas
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	LiftedStateModel model(problem);
	model.set_handlers(LiftedActionCSP::create_derived(problem.getPartiallyGroundedActions(), problem.get_tuple_index(), false, false));
	return model;
}


ExitCode 
SmartLiftedDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	LiftedStateModel model = setup(problem);
	SearchStats stats;
	auto engine = create(config, model, stats);
	return Utils::do_search(*engine, model, out_dir, start_time, stats);
}

} } // namespaces
