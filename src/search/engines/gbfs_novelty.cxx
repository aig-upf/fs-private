
#include <search/engines/gbfs_novelty.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <actions/applicable_action_set.hxx>

namespace fs0 { namespace engines {
	
std::unique_ptr<FS0SearchAlgorithm> GBFSNoveltyEngineCreator::create(const Config& config, const FS0StateModel& model) const {
	FS0SearchAlgorithm* engine = nullptr;
	
	unsigned max_novelty = config.getOption<int>("engine.max_novelty");
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	NoveltyHeuristic evaluator(model, max_novelty, feature_configuration);
	engine = new aptk::StlBestFirstSearch<SearchNode, NoveltyHeuristic, FS0StateModel>(model, std::move(evaluator));
	
	FINFO("main", "Heuristic options:");
	FINFO("main", "\tMax novelty: " << max_novelty);
	FINFO("main", "\tFeatiue extaction: " << feature_configuration);
	
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

} } // namespaces
