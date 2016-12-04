
#include <search/drivers/iterated_width.hxx>
#include <search/algorithms/iterated_width.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>


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
IteratedWidthDriver::setup(const Config& config, Problem& problem) const {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem); // By default we ground all actions and return a model with the problem as it is
}


} } // namespaces
