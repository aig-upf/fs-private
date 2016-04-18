
#include <search/drivers/fully_lifted_driver.hxx>
#include <problem.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <constraints/gecode/handlers/lifted_formula_handler.hxx>

#include <state.hxx>
#include <actions/lifted_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <utils/support.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace drivers {
std::unique_ptr<aptk::SearchAlgorithm<LiftedStateModel>> FullyLiftedDriver::create(const Config& config, LiftedStateModel& model) const {
	const Problem& problem = model.getTask();
	
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	const std::vector<const PartiallyGroundedAction*>& actions = problem.getPartiallyGroundedActions();
	auto managers = ActionSchemaCSPHandler::create(actions, problem.get_tuple_index(), approximate, novelty);
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	
	GecodeCRPG heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	return std::unique_ptr<LiftedEngine>(new aptk::StlBestFirstSearch<SearchNode, GecodeCRPG, LiftedStateModel>(model, std::move(heuristic)));
}


LiftedStateModel FullyLiftedDriver::setup(const Config& config, Problem& problem) const {
	// We don't ground any action
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	LiftedStateModel model(problem);
	model.set_handlers(ActionSchemaCSPHandler::create_derived(problem.getPartiallyGroundedActions(), problem.get_tuple_index(), false, false));
	return model;
}


} } // namespaces
