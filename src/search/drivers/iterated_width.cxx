
#include <search/drivers/iterated_width.hxx>
#include <search/algorithms/iterated_width.hxx>
#include <search/utils.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <search/drivers/setups.hxx>


namespace fs0 { namespace drivers {

template <typename StateModelT>
typename IteratedWidthDriver<StateModelT>::EnginePtr
IteratedWidthDriver<StateModelT>::create(const Config& config, const StateModelT& model) {
	
	unsigned max_novelty = config.getOption<int>("width.max_novelty");
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	LPT_INFO("main", "Heuristic options:");
	LPT_INFO("main", "\tMax novelty: " << max_novelty);
	LPT_INFO("main", "\tFeature extraction: " << feature_configuration);
	
	
	return EnginePtr(new Engine(model, 1, max_novelty, feature_configuration, _stats));
}

template <>
void 
IteratedWidthDriver<GroundStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	auto model = GroundingSetup::fully_ground_model(problem);
	auto engine = create(config, model);
	Utils::do_search(*engine, model, out_dir, start_time, _stats);
}

template <>
void 
IteratedWidthDriver<LiftedStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	auto model = GroundingSetup::fully_lifted_model(problem);
	auto engine = create(config, model);
	Utils::do_search(*engine, model, out_dir, start_time, _stats);
}

} } // namespaces
