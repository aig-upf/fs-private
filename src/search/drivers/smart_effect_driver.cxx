
#include <search/drivers/smart_effect_driver.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <actions/applicable_action_set.hxx>
#include <actions/grounding.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {

std::unique_ptr<FS0SearchAlgorithm> SmartEffectDriver::create(const Config& config, const FS0StateModel& model) const {
	FINFO("main", "Using the lifted-effect base RPG constructor");
	const Problem& problem = model.getTask();
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	
	SmartRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints());
	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& base_actions = problem.getPartiallyGroundedActions();
	std::vector<std::shared_ptr<EffectSchemaCSPHandler>> managers = EffectSchemaCSPHandler::create_smart(base_actions, tuple_index, approximate, novelty);
	heuristic.set_managers(std::move(managers)); // TODO Probably we don't need this to be shared_ptr's anymore
	
	return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, SmartRPG, FS0StateModel>(model, std::move(heuristic)));
}

void SmartEffectDriver::setup(const Config& config, Problem& problem) const {
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	// etc.
}


} } // namespaces
