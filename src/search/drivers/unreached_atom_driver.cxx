
#include <search/drivers/unreached_atom_driver.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <actions/applicable_action_set.hxx>
#include <actions/grounding.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {

std::unique_ptr<FS0SearchAlgorithm> UnreachedAtomDriver::create(const Config& config, const GroundStateModel& model) const {
	FINFO("main", "Using the lifted-effect base RPG constructor");
	const Problem& problem = model.getTask();
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	
	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const GroundAction*>& base_actions = problem.getGroundActions();
	UnreachedAtomRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), GroundEffectCSPHandler::create(base_actions, tuple_index, approximate, novelty));
	
	return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, UnreachedAtomRPG, GroundStateModel>(model, std::move(heuristic)));
}

GroundStateModel UnreachedAtomDriver::setup(const Config& config, Problem& problem) const {
	// We ground all actions
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem);
}

} } // namespaces
