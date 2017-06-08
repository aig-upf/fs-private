
#include <search/drivers/gbfs_constrained.hxx>
#include <constraints/direct/action_manager.hxx>
#include <search/drivers/validation.hxx>
#include "setups.hxx"
#include <lapkt/algorithms/best_first_search.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <constraints/gecode/handlers/ground_action_csp.hxx>
#include <languages/fstrips/formulae.hxx>
#include <utils/support.hxx>
#include <search/stats.hxx>
#include <search/utils.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace drivers {

GBFS_CRPGDriver::~GBFS_CRPGDriver() {
	delete _heuristic;
}

ExitCode
GBFS_CRPGDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	
	const auto model = drivers::GroundingSetup::fully_ground_model(problem);
	const std::vector<const GroundAction*>& actions = problem.getGroundActions();
	
	bool novelty = config.useNoveltyConstraint();
	bool approximate = config.useApproximateActionResolution();
	
	LPT_INFO("main", "Chosen CSP Manager: Gecode");
	
	Validation::check_no_conditional_effects(problem);
	auto managers = GroundActionCSP::create(actions, problem.get_tuple_index(), approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	
	if (config.getHeuristic() == "hff") {
		_heuristic = new GecodeCRPG(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	} else {
		assert(config.getHeuristic() == "hmax");
		_heuristic = new GecodeCHMax(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	}
	
	using EngineT = lapkt::StlBestFirstSearch<NodeT, GroundStateModel>;
	auto engine = std::unique_ptr<EngineT>(new EngineT(model));
	
	SearchStats stats;
	
	drivers::EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	drivers::EventUtils::setup_evaluation_observer<NodeT, GecodeCRPG>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	return drivers::Utils::do_search(*engine, model, out_dir, start_time, stats);
}


} } // namespaces
