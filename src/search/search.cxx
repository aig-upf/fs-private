

#include <time.h>

#include <aptk2/tools/resources_control.hxx>

#include <problem.hxx>
#include <search/search.hxx>
#include <search/engines/registry.hxx>
#include <search/engines/gbfs_crpg_lifted.hxx>
#include <actions/checker.hxx>
#include <utils/printers/printers.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <languages/fstrips/language.hxx>
#include <lifted_state_model.hxx>
#include <state.hxx>


namespace fs0 { namespace engines {


template <typename StateModelT, typename SearchAlgorithmT>
float SearchUtils::do_search(SearchAlgorithmT& engine, const StateModelT& model, const std::string& out_dir) {
	const Problem& problem = model.getTask();

	std::cout << "Writing results to directory: " << out_dir << std::endl;
	std::ofstream plan_out(out_dir + "/first.plan");
	std::ofstream json_out( out_dir + "/results.json" );

	std::vector<typename StateModelT::ActionType::IdType> plan;
	float t0 = aptk::time_used();
	double _t0 = (double) clock() / CLOCKS_PER_SEC;
	bool solved = engine.solve_model( plan );
	float total_time = aptk::time_used() - t0;
	double _total_time = (double) clock() / CLOCKS_PER_SEC - _t0;

	bool valid = false;
	
	if ( solved ) {
		PlanPrinter::print(plan, plan_out);
		valid = Checker::check_correctness(problem, plan, problem.getInitialState());
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
	

	return total_time;
}

void SearchUtils::instantiate_seach_engine_and_run(const Problem& problem, const Config& config, int timeout, const std::string& out_dir) {
	float timer = 0.0;
	std::cout << "Starting search with Relaxed Plan Heuristic and GBFS (time budget is " << timeout << " secs)..." << std::endl;
	
	// The engine and search model for lifted planning are different!
	if (config.doLiftedPlanning()) {

		fs0::LiftedStateModel model(problem);
		GBFSLiftedPlannerCreator creator;
		auto engine = creator.create(config, model);
		timer = do_search<>(*engine, model, out_dir);

	} else {
		// Standard, grounded planning
		FS0StateModel model(problem);
		auto creator = fs0::engines::EngineRegistry::instance().get(config.getEngineTag());
		auto engine = creator->create(config, model);
		timer = do_search(*engine, model, out_dir);
	}
	
	std::cout << "Search completed in " << timer << " secs" << std::endl;
}

void SearchUtils::report_stats(const Problem& problem) {
	auto actions = problem.getGroundActions();
	unsigned n_actions = actions.size();
	
	std::cout << "Number of objects: " << problem.getProblemInfo().getNumObjects() << std::endl;
	std::cout << "Number of state variables: " << problem.getProblemInfo().getNumVariables() << std::endl;
	std::cout << "Number of action schemata: " << problem.getActionSchemata().size() << std::endl;
	std::cout << "Number of ground actions: " << n_actions << std::endl;
	
	if (n_actions > 1000) {
		std::cout << "WARNING: The number of ground actions (" << n_actions << ") is too high for our applicable action strategy to perform well." << std::endl;
	}

	std::cout << "Number of state constraints: " << problem.getStateConstraints()->all_atoms().size() << std::endl;
	std::cout << "Number of goal conditions: " << problem.getGoalConditions()->all_atoms().size() << std::endl;
}

} } // namespaces
