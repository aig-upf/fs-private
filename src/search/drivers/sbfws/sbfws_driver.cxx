
#include <search/drivers/sbfws/sbfws_driver.hxx>
#include <search/utils.hxx>
#include <aptk2/tools/logging.hxx>

namespace fs0 { namespace bfws {

SBFWSConfig::SBFWSConfig(const Config& config) :
	search_width(config.getOption<int>("width.search")),
	simulation_width(config.getOption<int>("width.simulation"))
{}


template <>
ExitCode 
SimulatedBFWSDriver<GroundStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	return do_search(drivers::GroundingSetup::fully_ground_model(problem), config, out_dir, start_time);
}

template <>
ExitCode 
SimulatedBFWSDriver<LiftedStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	return do_search(drivers::GroundingSetup::fully_lifted_model(problem), config, out_dir, start_time);
}

template <typename StateModelT>
ExitCode
SimulatedBFWSDriver<StateModelT>::do_search(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time) {
	SBFWSConfig bfws_config(config);
	NoveltyFeaturesConfiguration feature_configuration(config);
	auto engine = create(config, bfws_config, feature_configuration, model);
	LPT_INFO("cout", "Simulated BFWS Configuration:");
	LPT_INFO("cout", "\tMaximum search novelty: " << bfws_config.search_width);
	LPT_INFO("cout", "\tMaximum simulation novelty: " << bfws_config.simulation_width);
// 	LPT_INFO("cout", "\tFeature extraction: " << feature_configuration);
	return drivers::Utils::do_search(*engine, model, out_dir, start_time, getStats());
}

} } // namespaces
