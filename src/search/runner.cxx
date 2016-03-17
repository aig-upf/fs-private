
#include <aptk2/tools/resources_control.hxx>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <search/search.hxx>

#include <search/runner.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>


namespace fs0 { namespace engines {

Runner::Runner(const EngineOptions& options, ProblemGeneratorType generator) 
	: _options(options), _generator(generator), _start_time(aptk::time_used())
{
	Logger::init(_options.getOutputDir() + "/logs");
	Config::init(_options.getConfig());

	FINFO("main", "Generating the problem (" << _options.getDataDir() << ")... ");
	auto data = Loader::loadJSONObject(_options.getDataDir() + "/problem.json");
	
	//! This will generate the problem and set it as the global singleton instance
	_generator(data, _options.getDataDir());
}

int Runner::run() {
	const Problem& problem = Problem::getInstance();
	const Config& config = Config::instance();
	
	FINFO("main", "Problem instance loaded:" << std::endl << problem);
	SearchUtils::report_stats(problem);
	
	FINFO("main", "Planner configuration: " << std::endl << config);
	SearchUtils::instantiate_seach_engine_and_run(problem, config, _options.getOutputDir(), _start_time);
	return 0;
}

} } // namespaces
