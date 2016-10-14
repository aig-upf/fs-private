
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
	report_stats(*problem, _options.getOutputDir());
	
	LPT_INFO("main", "Planner configuration: " << std::endl << config);
	LPT_INFO("cout", "Deriving control to search engine...");
	auto driver = EngineRegistry::instance().get(_options.getDriver());
	driver->search(*problem, config, _options.getOutputDir(), _start_time);
	
	return 0;
}

void Runner::report_stats(const Problem& problem, const std::string& out_dir) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	unsigned n_actions = problem.getGroundActions().size();
	std::ofstream json_out( out_dir + "/problem_stats.json" );
	json_out << "{" << std::endl;
	
	LPT_INFO("cout", "Number of objects: " << info.getNumObjects());
	LPT_INFO("cout", "Number of state variables: " << info.getNumVariables());
	LPT_INFO("cout", "Number of action schemata: " << problem.getActionData().size());
	LPT_INFO("cout", "Number of (perhaps partially) ground actions: " << n_actions);
	LPT_INFO("cout", "Number of goal atoms: " << problem.getGoalConditions()->all_atoms().size());
	LPT_INFO("cout", "Number of state constraint atoms: " << problem.getStateConstraints()->all_atoms().size());
	

	json_out << "\t\"num_objects\": " << info.getNumObjects() << "," << std::endl;
	json_out << "\t\"num_state_variables\": " << info.getNumVariables() << "," << std::endl;
	json_out << "\t\"num_action_schema\": " << problem.getActionData().size() << "," << std::endl;
	json_out << "\t\"num_grounded_actions\": " << n_actions << "," << std::endl;
	json_out << "\t\"num_goal_atoms\": " << problem.getGoalConditions()->all_atoms().size() << "," << std::endl;
	json_out << "\t\"num_state_constraint_atoms\": " << problem.getStateConstraints()->all_atoms().size();
	json_out << std::endl << "}" << std::endl;
	json_out.close();
}

} } // namespaces
