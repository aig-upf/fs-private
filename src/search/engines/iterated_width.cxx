
#include <search/engines/iterated_width.hxx>
#include <search/components/novelty_evaluator.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>

namespace fs0 { namespace engines {
	
std::unique_ptr<FS0SearchAlgorithm> IteratedWidthEngineCreator::create(const Config& config, const FS0StateModel& model) const {
	FS0SearchAlgorithm* engine = nullptr;
	
	unsigned max_novelty = config.getOption<int>("engine.max_novelty");
	bool use_state_vars = config.getOption<bool>("engine.use_state_vars");
	bool use_goal = config.getOption<bool>("engine.use_goal");
	bool use_actions = config.getOption<bool>("engine.use_actions");
	
	std::shared_ptr<SearchNoveltyEvaluator> evaluator = std::make_shared<SearchNoveltyEvaluator>(model);
	evaluator->setup(max_novelty, use_state_vars, use_goal, use_actions);
	
	FINFO("main", "Heuristic options:");
	FINFO("main", "\tMax novelty: " << max_novelty);
	FINFO("main", "\tUsing state vars as features: " << ( use_state_vars ? "yes" : "no "));
	FINFO("main", "\tUsing goal as feature: " << ( use_goal ? "yes" : "no "));
	FINFO("main", "\tUsing actions as features: " << ( use_actions ? "yes" : "no "));
	
	engine = new aptk::StlBreadthFirstSearch<SearchNode, FS0StateModel, OpenList>(model, OpenList(evaluator));
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

} } // namespaces
