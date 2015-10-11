

#include <aptk2/tools/resources_control.hxx>

#include <problem.hxx>
#include <search/search.hxx>
#include <search/engines/registry.hxx>
#include <actions/checker.hxx>
#include <utils/printers/printers.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>

#include <time.h>

namespace fs0 { namespace engines {

bool SearchUtils::check_plan(const std::vector<GroundAction::IdType>& plan) {
	const Problem& problem = Problem::getInstance();
	std::vector<unsigned> p;
	for (const auto& elem:plan) p.push_back((unsigned) elem);
	return Checker::checkPlanSuccessful(problem, p, *(problem.getInitialState()));
}

float SearchUtils::do_search(fs0::engines::FS0SearchAlgorithm& engine, const Problem& problem, const std::string& out_dir) {

	std::cout << "Writing results to " << out_dir << std::endl;
	std::ofstream plan_out(out_dir + "/first.plan");
	std::ofstream json_out( out_dir + "/results.json" );

	std::vector<GroundAction::IdType> plan;
	float t0 = aptk::time_used();
	double _t0 = (double) clock() / CLOCKS_PER_SEC;
	bool solved = engine.solve_model( plan );
	float total_time = aptk::time_used() - t0;
	double _total_time = (double) clock() / CLOCKS_PER_SEC - _t0;

	bool valid = check_plan(plan);
	if ( solved ) {
		PlanPrinter::printPlan(plan, problem, plan_out);
	}
	plan_out.close();

	std::string eval_speed = (total_time > 0) ? std::to_string((float) engine.generated / total_time) : "0";
	json_out << "{" << std::endl;
	json_out << "\t\"search_time\": " << total_time << "," << std::endl;
	json_out << "\t\"search_time_alt\": " << _total_time << "," << std::endl;
	json_out << "\t\"generated\": " << engine.generated << "," << std::endl;
	json_out << "\t\"expanded\": " << engine.expanded << "," << std::endl;
	json_out << "\t\"eval_per_second\": " << eval_speed << "," << std::endl;
	json_out << "\t\"solved\": " << ( solved ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"valid\": " << ( valid ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"plan_length\": " << plan.size() << "," << std::endl;
	json_out << "\t\"plan\": ";
	if ( solved )
		PlanPrinter::printPlanJSON( plan, problem, json_out);
	else
		json_out << "null";
	json_out << std::endl;
	json_out << "}" << std::endl;
	json_out.close();

	return total_time;
}

void SearchUtils::instantiate_seach_engine_and_run(const Problem& problem, const Config& config, int timeout, const std::string& out_dir) {
	float timer = 0.0;
	std::cout << "Starting search with Relaxed Plan Heuristic and GBFS (time budget is " << timeout << " secs)..." << std::endl;
	FS0StateModel model(problem);
	auto creator = fs0::engines::EngineRegistry::instance().get(config.getEngineTag());
	auto engine = creator->create(config, model);
	timer = do_search(*engine, problem, out_dir);
	std::cout << "Search completed in " << timer << " secs" << std::endl;
}

void SearchUtils::report_stats(const Problem& problem) {
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

} } // namespaces
