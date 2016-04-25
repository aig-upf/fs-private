
#include <search/drivers/gbfs_novelty.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <actions/ground_action_iterator.hxx>

namespace fs0 { namespace drivers {
	
std::unique_ptr<FS0SearchAlgorithm> GBFSNoveltyDriver::create(const Config& config, const GroundStateModel& model) const {
	FS0SearchAlgorithm* engine = nullptr;
	
	unsigned max_novelty = config.getOption<int>("engine.max_novelty");
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	NoveltyHeuristic evaluator(model, max_novelty, feature_configuration);
	engine = new aptk::StlBestFirstSearch<SearchNode, NoveltyHeuristic, GroundStateModel>(model, std::move(evaluator));
	
	FINFO("main", "Heuristic options:");
	FINFO("main", "\tMax novelty: " << max_novelty);
	FINFO("main", "\tFeatiue extaction: " << feature_configuration);
	
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

} } // namespaces
