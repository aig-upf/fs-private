
#include <engines/factory.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <heuristics/rpg/action_manager_factory.hxx>
#include <problem.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace engines {

	std::unique_ptr<FS0SearchAlgorithm> EngineFactory::create(const Config& config, const FS0StateModel& model) {
		
		// TODO - Instantiate the search engine based on the configuration object
		
		const Problem& problem = model.getTask();
		
		auto managers = ActionManagerFactory::create(problem.getGroundActions());
		std::shared_ptr<DirectRPGBuilder> direct_builder = std::shared_ptr<DirectRPGBuilder>(DirectRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints()));
		RelaxedPlanHeuristic<FS0StateModel, DirectRPGBuilder> heuristic_1(model, std::move(managers), direct_builder);
		
		auto engine = new aptk::StlBestFirstSearch<FS0SearchNode, RelaxedPlanHeuristic<FS0StateModel, DirectRPGBuilder>, FS0StateModel>(model, std::move(heuristic_1));
		
		
		/*
		std::shared_ptr<GecodeRPGBuilder> gecode_builder = std::shared_ptr<GecodeRPGBuilder>(GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints()));
		RelaxedPlanHeuristic<FS0StateModel, GecodeRPGBuilder> heuristic_2(model, managers, gecode_builder);
		*/
		
		return std::unique_ptr<FS0SearchAlgorithm>(engine);
	}
} } // namespaces
