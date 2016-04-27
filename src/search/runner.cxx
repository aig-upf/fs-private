
#include <aptk2/tools/resources_control.hxx>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <search/search.hxx>

#include <search/runner.hxx>
#include <utils/config.hxx>
#include <aptk2/tools/logging.hxx>


namespace fs0 { namespace drivers {

Runner::Runner(const EngineOptions& options, ProblemGeneratorType generator) 
	: _options(options), _generator(generator), _start_time(aptk::time_used())
{}

int Runner::run() {
	aptk::Logger::init(_options.getOutputDir() + "/logs");
	Config::init(_options.getConfig());

	std::cout << "Loading problem data" << std::endl;
	//! This will generate the problem and set it as the global singleton instance
	auto data = Loader::loadJSONObject(_options.getDataDir() + "/problem.json");
	Problem* problem = _generator(data, _options.getDataDir());
	const Config& config = Config::instance();
	
	LPT_INFO("main", "Problem instance loaded:" << std::endl << *problem);
	SearchUtils::report_stats(*problem);
	
	LPT_INFO("main", "Planner configuration: " << std::endl << config);
	SearchUtils::instantiate_seach_engine_and_run(*problem, config, _options.getDriver(), _options.getOutputDir(), _start_time);
	return 0;
}

} } // namespaces
