
#include <search/engines/gbfs_crpg_lifted.hxx>
#include <problem.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <constraints/gecode/handlers/ground_action_handler.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <state.hxx>
#include <actions/lifted_action_iterator.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace engines {
std::unique_ptr<aptk::SearchAlgorithm<LiftedStateModel>> GBFSLiftedPlannerCreator::create(const Config& config, LiftedStateModel& model) const {
	
	const Problem& problem = model.getTask();
	
	// The CSP handlers for applicable action iteration: we do not need novelty constraints (because they are instantiated on a standard state, not on a RPG layer),
	// and we need full resolution
	model.set_handlers(ActionSchemaCSPHandler::create_derived(problem.getActionSchemata(), false, false, false));
	
	
	if (Config::instance().getCSPModel() != Config::CSPModel::ActionSchemaCSP) {
		std::cout << "WARNING: Lifted planning overrides the CSP model option. Enforcing the action-schema CSP model." << std::endl;
	}
	
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	bool dont_care = Config::instance().useElementDontCareOptimization();
	
	auto gecode_builder = GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints());
	
	std::vector<std::shared_ptr<BaseActionCSPHandler>> csp_handlers = ActionSchemaCSPHandler::create(problem.getActionSchemata(), approximate, novelty, dont_care);
	
	GecodeCRPG gecode_builder_heuristic(problem, std::move(csp_handlers), std::move(gecode_builder));
	LiftedEngine* engine = new aptk::StlBestFirstSearch<SearchNode, GecodeCRPG, LiftedStateModel>(model, std::move(gecode_builder_heuristic));
	return std::unique_ptr<LiftedEngine>(engine);
}






} } // namespaces
