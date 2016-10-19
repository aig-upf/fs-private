
#include <search/drivers/unreached_atom_driver.hxx>
#include <search/utils.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>
#include <constraints/gecode/handlers/ground_effect_csp.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <utils/support.hxx>
#include <search/drivers/setups.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace drivers {

UnreachedAtomDriver::EnginePT
UnreachedAtomDriver::create(const Config& config, const GroundStateModel& model, SearchStats& stats) {
	LPT_INFO("main", "Using the lifted-effect base RPG constructor");
	const Problem& problem = model.getTask();
	bool novelty = config.useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = config.useApproximateActionResolution();

	
	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const GroundAction*>& actions = problem.getGroundActions();
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(
		problem, problem.getGoalConditions(), problem.getStateConstraints(),
		GroundEffectCSP::create(actions, tuple_index, approximate, novelty),
		extension_handler)
	);
	
	auto engine = EnginePT(new EngineT(model, *_heuristic));
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return engine;
}

GroundStateModel
UnreachedAtomDriver::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}


ExitCode 
UnreachedAtomDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	GroundStateModel model = setup(problem);
	SearchStats stats;
	auto engine = create(config, model, stats);
	return Utils::do_search(*engine, model, out_dir, start_time, stats);
}

} } // namespaces
