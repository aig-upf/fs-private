

#include <time.h>

#include <aptk2/tools/resources_control.hxx>

#include <problem.hxx>
#include <search/search.hxx>
#include <search/drivers/registry.hxx>
#include <search/drivers/fully_lifted_driver.hxx>
#include "drivers/smart_lifted_driver.hxx"
#include <actions/checker.hxx>
#include <utils/printers/printers.hxx>
#include <languages/fstrips/language.hxx>
#include <state.hxx>


namespace fs0 { namespace drivers {


template <typename StateModelT, typename SearchAlgorithmT>
void SearchUtils::do_search(SearchAlgorithmT& engine, const StateModelT& model, const std::string& out_dir, float start_time) {
	const Problem& problem = model.getTask();

	std::cout << "Writing results to directory: " << out_dir << std::endl;
	std::ofstream plan_out(out_dir + "/first.plan");
	std::ofstream json_out( out_dir + "/results.json" );

	std::vector<typename StateModelT::ActionType::IdType> plan;
	float t0 = aptk::time_used();
	double _t0 = (double) clock() / CLOCKS_PER_SEC;
	
	bool solved = engine.solve_model( plan );
	
	float search_time = aptk::time_used() - t0;
	double _search_time = (double) clock() / CLOCKS_PER_SEC - _t0;
	float total_planning_time = aptk::time_used() - start_time;

	bool valid = false;
	
	if ( solved ) {
		PlanPrinter::print(plan, plan_out);
		valid = Checker::check_correctness(problem, plan, problem.getInitialState());
	}
	plan_out.close();

	std::string eval_speed = (search_time > 0) ? std::to_string((float) engine.generated / search_time) : "0";
	json_out << "{" << std::endl;
	json_out << "\t\"total_time\": " << total_planning_time << "," << std::endl;
	json_out << "\t\"search_time\": " << search_time << "," << std::endl;
	json_out << "\t\"search_time_alt\": " << _search_time << "," << std::endl;
	json_out << "\t\"generated\": " << engine.generated << "," << std::endl;
	json_out << "\t\"expanded\": " << engine.expanded << "," << std::endl;
	json_out << "\t\"eval_per_second\": " << eval_speed << "," << std::endl;
	json_out << "\t\"solved\": " << ( solved ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"valid\": " << ( valid ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"plan_length\": " << plan.size() << "," << std::endl;
	json_out << "\t\"plan\": ";
	PlanPrinter::print_json( plan, json_out);
	json_out << std::endl;
	json_out << "}" << std::endl;
	json_out.close();
	
	if (solved) {
		if (!valid) throw std::runtime_error("The plan output by the planner is not correct!");
		std::cout << "Search Result: Found plan of length " << plan.size() << std::endl;
		std::cout << "Expanded / Evaluated / Eval. rate: " << engine.expanded << " / " << engine.generated << " / " << eval_speed << std::endl;
	} else {
		std::cout << "Search Result: No plan was found " << std::endl;
		// TODO - Make distinction btw all nodes explored and no plan found, and no plan found in the given time.
	}

	std::cout << "Total Planning Time: " << total_planning_time << " s." << std::endl;
	std::cout << "Actual Search Time: " << search_time << " s." << std::endl;
}

void SearchUtils::instantiate_seach_engine_and_run(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	std::cout << "Starting search..." << std::endl;
	
	const std::string& driver_tag = config.getEngineTag();
	
	// The engine and search model for lifted planning are different!
	// TODO - REFACTOR THIS
	if (driver_tag == "fully_lifted") {
		FullyLiftedDriver driver;
		fs0::LiftedStateModel model = driver.setup(config, problem);
		auto engine = driver.create(config, model);
		do_search(*engine, model, out_dir, start_time);

	} else if (driver_tag == "smart_lifted") {
		
		SmartLiftedDriver driver;
		fs0::LiftedStateModel model = driver.setup(config, problem);
		auto engine = driver.create(config, model);
		do_search(*engine, model, out_dir, start_time);
		
	} else {
		// Standard, grounded planning
		
		auto driver = fs0::drivers::EngineRegistry::instance().get(driver_tag);
		GroundStateModel model = driver->setup(config, problem);
		auto engine = driver->create(config, model);
		do_search(*engine, model, out_dir, start_time);
	}
}

void SearchUtils::report_stats(const Problem& problem) {
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
