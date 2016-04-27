
#include <search/drivers/iterated_width.hxx>
#include <search/algorithms/iterated_width.hxx>
#include <actions/applicable_action_set.hxx>

namespace fs0 { namespace drivers {

std::unique_ptr<FS0SearchAlgorithm> IteratedWidthDriver::create(const Config& config, const GroundStateModel& model) const {
	
	unsigned max_novelty = config.getOption<int>("engine.max_novelty");
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	LPT_INFO("main", "Heuristic options:");
	LPT_INFO("main", "\tMax novelty: " << max_novelty);
	LPT_INFO("main", "\tFeatiue extaction: " << feature_configuration);
	
	FS0SearchAlgorithm* engine = new FS0IWAlgorithm(model, 1, max_novelty, feature_configuration);
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

} } // namespaces
