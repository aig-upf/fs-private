
#include <aptk2/tools/resources_control.hxx>
#include <aptk2/tools/logging.hxx>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <search/runner.hxx>
#include <search/drivers/registry.hxx>
#include <utils/config.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/language.hxx>


namespace fs0 { namespace drivers {

Runner::Runner(const EngineOptions& options, ProblemGeneratorType generator) 
	: _options(options), _generator(generator), _start_time(aptk::time_used())
{}

int Runner::run() {
	aptk::Logger::init(_options.getOutputDir() + "/logs");
	Config::init(_options.getDriver(), _options.getUserOptions(), _options.getDefaultConfigurationFilename());

	std::cout << "Loading problem data" << std::endl;
	//! This will generate the problem and set it as the global singleton instance
	auto data = Loader::loadJSONObject(_options.getDataDir() + "/problem.json");
	Problem* problem = _generator(data, _options.getDataDir());
	const Config& config = Config::instance();
	
	LPT_INFO("main", "Problem instance loaded:" << std::endl << *problem);
	report_stats(*problem);
	
	LPT_INFO("main", "Planner configuration: " << std::endl << config);
	LPT_INFO("cout", "Deriving control to search engine...");
	auto driver = EngineRegistry::instance().get(_options.getDriver());
	driver->search(*problem, config, _options.getOutputDir(), _start_time);
	
	return 0;
}

void Runner::report_stats(const Problem& problem) {
	auto actions = problem.getGroundActions();
	unsigned n_actions = actions.size();
	
	std::cout << "Number of objects: " << ProblemInfo::getInstance().getNumObjects() << std::endl;
	std::cout << "Number of state variables: " << ProblemInfo::getInstance().getNumVariables() << std::endl;
	std::cout << "Number of action schemata: " << problem.getActionData().size() << std::endl;
	std::cout << "Number of (perhaps partially) ground actions: " << n_actions << std::endl;
	
	if (n_actions > 1000) {
		std::cout << "WARNING: The number of ground actions (" << n_actions << ") is too high for our applicable action strategy to perform well." << std::endl;
	}

	std::cout << "Number of state constraints: " << problem.getStateConstraints()->all_atoms().size() << std::endl;
	std::cout << "Number of goal conditions: " << problem.getGoalConditions()->all_atoms().size() << std::endl;
}

} } // namespaces
