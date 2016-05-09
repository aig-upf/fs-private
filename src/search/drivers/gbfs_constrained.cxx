
#include <search/drivers/gbfs_constrained.hxx>
#include <constraints/direct/action_manager.hxx>
#include <search/drivers/validation.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <constraints/gecode/handlers/ground_action_csp.hxx>
#include <constraints/gecode/handlers/ground_effect_csp.hxx>
#include <constraints/gecode/handlers/lifted_action_csp.hxx>
#include <actions/ground_action_iterator.hxx>
#include <languages/fstrips/formulae.hxx>
#include <utils/support.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace drivers {

std::unique_ptr<FS0SearchAlgorithm> GBFSConstrainedHeuristicsCreator::create(const Config& config, const GroundStateModel& model) const {
	const Problem& problem = model.getTask();
	const std::vector<const GroundAction*>& actions = problem.getGroundActions();
	
	bool novelty = config.useNoveltyConstraint();
	bool approximate = config.useApproximateActionResolution();
	bool delayed = config.useDelayedEvaluation();
	
	LPT_INFO("main", "Chosen CSP Manager: Gecode");
	
	Validation::check_no_conditional_effects(problem);
	auto managers = GroundActionCSP::create(actions, problem.get_tuple_index(), approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	if (config.getHeuristic() == "hff") {
		GecodeCRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
		return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, GecodeCRPG, GroundStateModel>(model, std::move(heuristic), delayed));
	} else {
		assert(config.getHeuristic() == "hmax");
		GecodeCHMax heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
		return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, GecodeCHMax, GroundStateModel>(model, std::move(heuristic), delayed));
	}
}



} } // namespaces
