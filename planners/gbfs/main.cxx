
#include <iostream>
#include <fstream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <aptk2/search/algorithms/best_first_search.hxx>
#include <aptk2/tools/resources_control.hxx>

#include <utils/logging.hxx>
#include <utils/loader.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <utils/printers/printers.hxx>
#include <utils/config.hxx>
#include <problem_info.hxx>
#include <actions/checker.hxx>
#include <state_model.hxx>
#include <engines/factory.hxx>

#include <problem.hxx>

#include <components.hxx>  // This will dinamically point to the right generated file

using namespace fs0;


// MRJ: We start defining the type of nodes for our planner
typedef std::vector<GroundAction::IdType> Plan;

bool checkPlanCorrect(const Plan& plan) {
	auto problem = Problem::getCurrentProblem();
	std::vector<unsigned> p;
	for (const auto& elem:plan) p.push_back((unsigned) elem);
	return Checker::checkPlanSuccessful(*problem, p, *(problem->getInitialState()));
}

float do_search(fs0::engines::FS0SearchAlgorithm& engine, const Problem& problem, const std::string& out_dir) {

	std::ofstream out(out_dir + "/searchlog.out");
	std::ofstream plan_out(out_dir + "/first.plan");
	std::ofstream json_out( out_dir + "/results.json" );

	std::cout << "Writing results to " << out_dir + "/searchlog.out" << std::endl;

	Plan plan;
	float t0 = aptk::time_used();
	bool solved = engine.solve_model( plan );
	float total_time = aptk::time_used() - t0;


	bool valid = checkPlanCorrect(plan);
	if ( solved ) {
		PlanPrinter::printPlan(plan, problem, out);
		PlanPrinter::printPlan(plan, problem, plan_out);
	}

	out << "Total time: " << total_time << std::endl;
	out << "Nodes generated during search: " << engine.generated << std::endl;
	out << "Nodes expanded during search: " << engine.expanded << std::endl;

	std::string eval_speed = (total_time > 0) ? std::to_string((float) engine.generated / total_time) : "-";
	out << "Heuristic evaluations per second: " <<  eval_speed << std::endl;

	out.close();
	plan_out.close();

	json_out << "{" << std::endl;
	json_out << "\tsearch_time : " << total_time << "," << std::endl;
	json_out << "\tgenerated : " << engine.generated << "," << std::endl;
	json_out << "\texpanded : " << engine.expanded << "," << std::endl;
	json_out << "\teval_per_second : " << eval_speed << "," << std::endl;
	json_out << "\tsolved : " << ( solved ? "true" : "false" ) << "," << std::endl;
	json_out << "\tvalid : " << ( valid ? "true" : "false" ) << "," << std::endl;
	json_out << "\tplan : ";
	if ( solved )
		PlanPrinter::printPlanJSON( plan, problem, json_out);
	else
		json_out << "null";
	json_out << std::endl;
	json_out << "}" << std::endl;

	json_out.close();

	return total_time;
}


void instantiate_seach_engine_and_run(const FS0StateModel& search_prob, int timeout, const std::string& out_dir) {
	float timer = 0.0;
	std::cout << "Starting search with Relaxed Plan Heuristic and GBFS (time budget is " << timeout << " secs)..." << std::endl;
	
	auto engine = fs0::engines::EngineFactory::create(Config::instance(), search_prob);
	timer = do_search(*engine, search_prob.getTask(), out_dir);
	std::cout << "Search completed in " << timer << " secs" << std::endl;
}


void reportProblemStats(const Problem& problem) {
	auto actions = problem.getGroundActions();
	unsigned n_actions = actions.size();
	
// 	std::cout << "Number of object types: " << st.get_num_types() << std::endl;
	std::cout << "Number of objects: " << problem.getProblemInfo().getNumObjects() << std::endl;
	std::cout << "Number of state variables: " << problem.getProblemInfo().getNumVariables() << std::endl;

	std::cout << "Number of ground actions: " << n_actions << std::endl;
	if (n_actions > 1000) {
		std::cout << "WARNING: The number of ground actions (" << n_actions <<
		") is too high for our current applicable action strategy to perform well." << std::endl;
	}

	std::cout << "Number of state constraints: " << problem.getStateConstraints().size() << std::endl;
	std::cout << "Number of goal conditions: " << problem.getGoalConditions().size() << std::endl;
}


namespace po = boost::program_options;

int parse_options(int argc, char** argv, int& timeout, std::string& data_dir, std::string& out_dir) {
	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "Display this help message")
		("timeout,t", po::value<int>()->default_value(10), "The timeout, in seconds.")
		("data", po::value<std::string>()->default_value("data"), "The directory where the input data is stored.")
		("out", po::value<std::string>()->default_value("."), "The directory where the results data is to be output.");

	po::positional_options_description pos;
	pos.add("timeout", 1)
	   .add("data", 1)
	   .add("out", 1);

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(description).positional(pos).run(), vm);
		po::notify(vm);
	} catch(const boost::program_options::invalid_option_value& ex) {
		std::cout << "Wrong parameter types:";
		std::cout << ex.what() << std::endl;
		std::cout << std::endl << description << std::endl;
		return 1;
	}

	if (vm.count("help")) {
		std::cout << description << "\n";
		return 1;
	}

	timeout = vm["timeout"].as<int>();
	data_dir = vm["data"].as<std::string>();
	out_dir = vm["out"].as<std::string>();
	return 0;
}


int main(int argc, char** argv) {

	int timeout; std::string data_dir; std::string out_dir;
	int res = parse_options(argc, argv, timeout, data_dir, out_dir);
	if (res != 0) {
		return res;
	}

	Logger::init("./logs");
	Config::init("config.json");
	
	FINFO("main", "Planner configuration: " << std::endl << Config::instance());
	FINFO("main", "Generating the problem (" << data_dir << ")... ");
	auto data = Loader::loadJSONObject(data_dir + "/problem.json");
	Problem problem;
	Problem::setCurrentProblem(problem);
	generate(data, data_dir, problem);
	problem.bootstrap();
	FINFO("main", "Problem bootstrapped" << std::endl << problem);
	
	reportProblemStats(problem);

	FS0StateModel search_prob(problem);

	std::cout << "Done!" << std::endl;

	// Instantiate the engine
	instantiate_seach_engine_and_run(search_prob, timeout, out_dir);
	return 0;
}
