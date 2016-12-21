
#pragma once

#include <string>

#include <aptk2/tools/resources_control.hxx>
#include <aptk2/tools/logging.hxx>

#include <fs_types.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <search/stats.hxx>
#include <actions/checker.hxx>
#include <utils/printers/printers.hxx>
#include <utils/system.hxx>


namespace fs0 { namespace drivers {

class Utils {
public:

template <typename StateModelT, typename SearchAlgorithmT, typename StatsT>
static ExitCode do_search(SearchAlgorithmT& engine, const StateModelT& model, const std::string& out_dir, float start_time, const StatsT& stats) {
	const Problem& problem = model.getTask();

	LPT_INFO("cout", "Starting search. Results written to " << out_dir);
	std::ofstream plan_out(out_dir + "/first.plan");
	std::ofstream json_out( out_dir + "/results.json" );

	std::vector<typename StateModelT::ActionType::IdType> plan;
	float t0 = aptk::time_used();
	
	bool solved = false, oom = false;
	try {
		solved = engine.solve_model( plan );
	}
	catch (const std::bad_alloc& ex)
	{
		LPT_INFO("cout", "FAILED TO ALLOCATE MEMORY");
		oom = true;
	}
	
	float search_time = aptk::time_used() - t0;
	float total_planning_time = aptk::time_used() - start_time;

	bool valid = false;
	
	if ( solved ) {
		PlanPrinter::print(plan, plan_out);
		valid = Checker::check_correctness(problem, plan, problem.getInitialState());
	}
	plan_out.close();

	std::string gen_speed = (search_time > 0) ? std::to_string((float) stats.generated() / search_time) : "0";
	std::string eval_speed = (search_time > 0) ? std::to_string((float) stats.evaluated() / search_time) : "0";
	

	json_out << "{" << std::endl;
	for (const auto& point:stats.dump()) {
		json_out << "\t\"" << std::get<0>(point) << "\": " << std::get<2>(point) << "," << std::endl;
	}
	json_out << "\t\"total_time\": " << total_planning_time << "," << std::endl;
	json_out << "\t\"search_time\": " << search_time << "," << std::endl;
	// json_out << "\t\"search_time_alt\": " << _search_time << "," << std::endl;
	json_out << "\t\"memory\": " << get_peak_memory_in_kb() << "," << std::endl;
	json_out << "\t\"gen_per_second\": " << gen_speed << "," << std::endl;
	json_out << "\t\"eval_per_second\": " << eval_speed << "," << std::endl;
	json_out << "\t\"solved\": " << ( solved ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"valid\": " << ( valid ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"out_of_memory\": " << ( oom ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"plan_length\": " << plan.size() << "," << std::endl;
	json_out << "\t\"plan\": ";
	PlanPrinter::print_json( plan, json_out);
	json_out << std::endl;
	json_out << "}" << std::endl;
	json_out.close();
	
	for (const auto& point:stats.dump()) {
		LPT_INFO("cout", std::get<1>(point) << ": " << std::get<2>(point));
	}
	LPT_INFO("cout", "Total Planning Time: " << total_planning_time << " s.");
	LPT_INFO("cout", "Actual Search Time: " << search_time << " s.");
	LPT_INFO("cout", "Peak mem. usage: " << get_peak_memory_in_kb() << " kB.");
	
	ExitCode result;
	if (solved) {
		if (!valid) {
			Checker::print_plan_execution(problem, plan, problem.getInitialState());
			throw std::runtime_error("The plan output by the planner is not correct!");
		}
		LPT_INFO("cout", "Search Result: Found plan of length " << plan.size());
		result = ExitCode::PLAN_FOUND;
	} else if (oom) {
		LPT_INFO("cout", "Search Result: Out of memory. Peak memory: " << get_peak_memory_in_kb());
		result = ExitCode::OUT_OF_MEMORY;
	} else {
		LPT_INFO("cout", "Search Result: No plan was found.");
		result = ExitCode::UNSOLVABLE;
	}
	
	return result;
}

};

} } // namespaces
