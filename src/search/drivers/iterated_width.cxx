
#include <search/drivers/iterated_width.hxx>
#include <search/algorithms/iterated_width.hxx>
#include <search/utils.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <search/drivers/setups.hxx>


namespace fs0 { namespace drivers {

std::unique_ptr<FSGroundSearchAlgorithm>
IteratedWidthDriver::create(const Config& config, const GroundStateModel& model) {
	
	unsigned max_novelty = config.getOption<int>("width.max_novelty");
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	LPT_INFO("main", "Heuristic options:");
	LPT_INFO("main", "\tMax novelty: " << max_novelty);
	LPT_INFO("main", "\tFeature extraction: " << feature_configuration);
	
	
	FSGroundSearchAlgorithm* engine = new FS0IWAlgorithm(model, 1, max_novelty, feature_configuration, _stats);
	return std::unique_ptr<FSGroundSearchAlgorithm>(engine);
}

GroundStateModel
IteratedWidthDriver::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}

void 
IteratedWidthDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	GroundStateModel model = setup(problem);
	auto engine = create(config, model);
	Utils::do_search(*engine, model, out_dir, start_time, _stats);
}


} } // namespaces
