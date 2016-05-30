
#include <search/drivers/smart_effect_driver.hxx>

#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <constraints/gecode/handlers/lifted_effect_csp.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>

#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/support.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {

SmartEffectDriver::Engine
SmartEffectDriver::create(const Config& config, const GroundStateModel& model) {
	LPT_INFO("main", "Using the smart-effect driver");
	const Problem& problem = model.getTask();
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();
	bool delayed = config.useDelayedEvaluation();
	
	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	auto managers = LiftedEffectCSP::create(actions, tuple_index, approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	SmartRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	
	// If necessary, we constrain the state variables domains and even action/effect CSPs that will be used henceforth
	// by performing a reachability analysis.
	if (config.getOption<bool>("reachability_analysis")) {
		LPT_INFO("main", "Applying reachability analysis");
		RPGIndex graph = heuristic.compute_full_graph(problem.getInitialState());
		LiftedEffectCSP::prune_unreachable(heuristic.get_managers(), graph);
	}
	
	EHCSearch<SmartRPG>* ehc = nullptr;
	if (config.getOption<bool>("ehc")) {
		// TODO Apply reachability analysis for the EHC heuristic as well
		auto ehc_managers = LiftedEffectCSP::create(actions, tuple_index, approximate, novelty);
		SmartRPG ehc_heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(ehc_managers), extension_handler);
		ehc = new EHCSearch<SmartRPG>(problem, std::move(ehc_heuristic));
	}
	
	FSGroundSearchAlgorithm* gbfs = new aptk::StlBestFirstSearch<SearchNode, SmartRPG, GroundStateModel>(model, std::move(heuristic), delayed);
	
	return Engine(new EHCThenGBFSSearch<SmartRPG>(problem, gbfs, ehc));
}

GroundStateModel
SmartEffectDriver::setup(const Config& config, Problem& problem) const {
	// We'll use all the ground actions for the search plus the partyally ground actions for the heuristic computations
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem);
}


} } // namespaces
