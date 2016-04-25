
#include <search/drivers/smart_lifted_driver.hxx>
#include <problem.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <constraints/gecode/handlers/formula_handler.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>

#include <state.hxx>
#include <actions/lifted_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <utils/support.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {
std::unique_ptr<aptk::SearchAlgorithm<LiftedStateModel>> SmartLiftedDriver::create(const Config& config, LiftedStateModel& model) const {
	const Problem& problem = model.getTask();
	
	bool novelty = Config::instance().useNoveltyConstraint() && !problem.is_predicative();
	bool approximate = Config::instance().useApproximateActionResolution();
	const auto& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	
	// We create smart managers by grounding only wrt the effect heads.
	std::vector<std::shared_ptr<EffectSchemaCSPHandler>> managers = EffectSchemaCSPHandler::create_smart(actions, tuple_index, approximate, novelty); // TODO Probably we don't need this to be shared_ptr's anymore
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	SmartRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	
	return std::unique_ptr<LiftedEngine>(new aptk::StlBestFirstSearch<SearchNode, SmartRPG, LiftedStateModel>(model, std::move(heuristic)));
}


LiftedStateModel SmartLiftedDriver::setup(const Config& config, Problem& problem) const {
	// We set up a lifted model with the action schemas
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	LiftedStateModel model(problem);
	model.set_handlers(ActionSchemaCSPHandler::create_derived(problem.getPartiallyGroundedActions(), problem.get_tuple_index(), false, false));
	return model;
}


} } // namespaces
