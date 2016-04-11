
#include <search/engines/gbfs_lifted_effect_crpg.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/lifted_crpg.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <actions/applicable_action_set.hxx>
#include <actions/actions.hxx>
#include <actions/grounding.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace engines {

std::unique_ptr<FS0SearchAlgorithm> GBFSLiftedEffectCRPG::create(const Config& config, const FS0StateModel& model) const {
	FINFO("main", "Using the lifted-effect base RPG constructor");
	const Problem& problem = model.getTask();
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	
	LiftedCRPG lifted_crpg(problem, problem.getGoalConditions(), problem.getStateConstraints());
	const TupleIndex& tuple_index = problem.get_tuple_index();
	const std::vector<const PartiallyGroundedAction*>& base_actions = problem.getPartiallyGroundedActions();
	std::vector<std::shared_ptr<EffectSchemaCSPHandler>> managers = EffectSchemaCSPHandler::create_smart(base_actions, tuple_index, approximate, novelty);
	lifted_crpg.set_managers(std::move(managers)); // TODO Probably we don't need this to be shared_ptr's anymore
	
	return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, LiftedCRPG, FS0StateModel>(model, std::move(lifted_crpg)));
}

void GBFSLiftedEffectCRPG::setup(const Config& config, Problem& problem) const {
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), Problem::getInfo()));
	// etc.
}


} } // namespaces
