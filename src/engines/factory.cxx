
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
		const Problem& problem = model.getTask();
		
		FS0SearchAlgorithm* engine = nullptr;
		
		auto action_managers = ActionManagerFactory::create(problem.getGroundActions());
		
		if (Config::instance().getGoalManagerType() == Config::GoalManagerType::Gecode) {
			// We use a Gecode RPG builder
			typedef RelaxedPlanHeuristic<FS0StateModel, GecodeRPGBuilder> GecodeRPHeuristic;
			std::shared_ptr<GecodeRPGBuilder> gecode_builder = std::shared_ptr<GecodeRPGBuilder>(GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints()));
			GecodeRPHeuristic gecode_builder_heuristic(model, std::move(action_managers), gecode_builder);
			engine = new aptk::StlBestFirstSearch<FS0SearchNode, GecodeRPHeuristic, FS0StateModel>(model, std::move(gecode_builder_heuristic));
			
		} else {
			// We use a direct RPG builder
			typedef RelaxedPlanHeuristic<FS0StateModel, DirectRPGBuilder> DirectRPHeuristic;
			std::shared_ptr<DirectRPGBuilder> direct_builder = std::shared_ptr<DirectRPGBuilder>(DirectRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints()));
			DirectRPHeuristic direct_builder_heuristic(model, std::move(action_managers), direct_builder);
			engine = new aptk::StlBestFirstSearch<FS0SearchNode, DirectRPHeuristic, FS0StateModel>(model, std::move(direct_builder_heuristic));
		}
		
		
		return std::unique_ptr<FS0SearchAlgorithm>(engine);
	}
} } // namespaces
