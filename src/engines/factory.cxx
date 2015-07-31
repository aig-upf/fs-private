
#include <engines/factory.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <problem.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>

namespace fs0 { namespace engines {

	std::unique_ptr<FS0SearchAlgorithm> EngineFactory::create(const Config& config, const FS0StateModel& model) {
		// TODO - Instantiate the search engine based on the configuration object
		
		const Problem& problem = model.getTask();
		
		
		DirectRPGBuilder::cptr direct_builder = new DirectRPGBuilder(problem.getGoalConditions(), problem.getStateConstraints());
		RelaxedPlanHeuristic<FS0StateModel, DirectRPGBuilder> heuristic_1(model, direct_builder);
		
		auto engine = new aptk::StlBestFirstSearch<FS0SearchNode, RelaxedPlanHeuristic<FS0StateModel, DirectRPGBuilder>, FS0StateModel>(model, heuristic_1);
		
		
		
		fs0::gecode::GecodeRPGBuilder::cptr gecode_builder = fs0::gecode::GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints());
		RelaxedPlanHeuristic<FS0StateModel, fs0::gecode::GecodeRPGBuilder> heuristic_2(model, gecode_builder);
		
		
		return std::unique_ptr<FS0SearchAlgorithm>(engine);
	}
} } // namespaces
