
#include <problem.hxx>
#include <utils/loader.hxx>
#include <search/search.hxx>

#include <search/runner.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>


namespace fs0 { namespace engines {

Runner::Runner(const EngineOptions& options, ProblemGeneratorType generator) 
	: _options(options), _generator(generator)
{
	Logger::init("./logs");
	Config::init("config.json");

	FINFO("main", "Generating the problem (" << _options.getDataDir() << ")... ");
	auto data = Loader::loadJSONObject(_options.getDataDir() + "/problem.json");
	
	
	Problem::setInstance(_generator(data, _options.getDataDir()));
}

int Runner::run() {
	const Problem& problem = Problem::getInstance();
	const Config& config = Config::instance();
	
	FINFO("main", "Problem instance loaded:" << std::endl << problem);
	SearchUtils::report_stats(problem);
	
	FINFO("main", "Planner configuration: " << std::endl << config);
	SearchUtils::instantiate_seach_engine_and_run(problem, config, _options.getTimeout(), _options.getOutputDir());
	return 0;
}

} } // namespaces
