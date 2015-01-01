#include <iostream>
#include <fstream>

#include <core_fwd_search_prob.hxx>

#include <aptk/open_list.hxx>
#include <aptk/at_gbfs.hxx>
#include <aptk/at_wbfs.hxx>

#include <heuristics/core_h_null.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <heuristics/hmax.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <utils/printers.hxx>
#include <problem_info.hxx>
#include <action_manager.hxx>

#include <generator.hxx>  // This will dinamically point to the right generated file

using	aptk::Action;

using 	aptk::search::Open_List;
using	aptk::search::Node_Comparer;
using	aptk::search::bfs::Node;

// MRJ: We start defining the type of nodes for our planner
typedef Node< aptk::core::State > Search_Node;

// MRJ: Then we define the type of the tie-breaking algorithm
// for the open list we are going to use
typedef		Node_Comparer< Search_Node >					Tie_Breaking_Algorithm;
typedef aptk::search::Node_Comparer_GBFS< Search_Node > GBFS_Comparer;


// MRJ: Now we define the Open List type by combining the types we have defined before
typedef		Open_List< GBFS_Comparer, Search_Node >		BFS_Open_List;


using namespace aptk::core;
// TODO - Wrap in namespace


// MRJ: Now we define the heuristics
// typedef		aptk::agnostic::H1_Heuristic<aptk::agnostic::Fwd_Search_Problem, aptk::agnostic::H_Add_Evaluation_Function>	H_Add_Fwd;
// typedef		Relaxed_Plan_Heuristic< aptk::agnostic::Fwd_Search_Problem, H_Add_Fwd >		H_Add_Rp_Fwd;
typedef		aptk::core::Null_Heuristic<aptk::core::FwdSearchProblem> NullHeuristic;
typedef		aptk::core::RelaxedPlanHeuristic<aptk::core::FwdSearchProblem> RelaxedHeuristic;
typedef		aptk::core::HMaxHeuristic<aptk::core::FwdSearchProblem> RelaxedMaxHeuristic;


// MRJ: Now we're ready to define the BFS algorithm we're going to use
// typedef		AT_BFS_SQ_SH< Fwd_Search_Problem, RelaxedHeuristic, BFS_Open_List >		Anytime_BFS_H_Add_Rp_Fwd;
typedef		aptk::search::bfs::AT_BFS_SQ_SH< aptk::core::FwdSearchProblem, NullHeuristic, BFS_Open_List > NullHeuristicSearchEngine;
typedef		aptk::search::bfs::AT_WBFS_SQ_SH< aptk::core::FwdSearchProblem, NullHeuristic, BFS_Open_List >	WeightedSearchEngine;


typedef std::vector<int> Plan;


bool checkPlanCorrect(const Plan& plan) {
	auto problem = aptk::core::Problem::getCurrentProblem();
	std::vector<unsigned> p;
	for (const auto& elem:plan) p.push_back((unsigned) elem);
	return aptk::core::ActionManager::checkPlanSuccessful(*problem, p, *(problem->getInitialState()));
}

template <typename Search_Engine>
float do_search( Search_Engine& engine, const ProblemInfo::cptr& problemInfo, float budget, const std::string& out_dir) {

	std::ofstream out(out_dir + "/searchlog.out");
	std::ofstream best_plan(out_dir + "/best.plan"),
				  first_plan(out_dir + "/first.plan");
	
	std::cout << "Writing results to " << out_dir + "/searchlog.out" << std::endl;

	engine.set_budget( budget );
	engine.start();

	Plan plan, last_plan;
	float cost;

	float ref = aptk::time_used();
	float t0 = aptk::time_used();

	unsigned expanded_0 = engine.expanded();
	unsigned generated_0 = engine.generated();
	
	bool found = false;

	// Deactivate anytime search
	if ( engine.find_solution( cost, plan ) ) {
// 	while ( engine.find_solution( cost, plan ) ) {
		assert(checkPlanCorrect(plan));
		out << "\n\nPlan found with cost: " << cost << std::endl;
		Printers::printPlan(plan, problemInfo, out);
		if (!found) {
			Printers::printPlan(plan, problemInfo, first_plan);
			found = true;
		}
		
		float tf = aptk::time_used();
		unsigned expanded_f = engine.expanded();
		unsigned generated_f = engine.generated();
		out << "Time: " << tf - t0 << std::endl;
		out << "Generated: " << generated_f - generated_0 << std::endl;
		out << "Expanded: " << expanded_f - expanded_0 << std::endl;
		t0 = tf;
		expanded_0 = expanded_f;
		generated_0 = generated_f;
		last_plan = plan;
		plan.clear();
	}
	
// 	Utils::printPlan(last_plan, problemInfo, best_plan); // Print the last plan
	
	float total_time = aptk::time_used() - ref;
	out << "Total time: " << total_time << std::endl;
	out << "Nodes generated during search: " << engine.generated() << std::endl;
	out << "Nodes expanded during search: " << engine.expanded() << std::endl;
	out << "Nodes pruned by bound: " << engine.pruned_by_bound() << std::endl;
	out << "Dead-end nodes: " << engine.dead_ends() << std::endl;
	out << "Nodes in OPEN replaced: " << engine.open_repl() << std::endl;

	out.close();
	best_plan.close(); first_plan.close();
	
	return total_time;
}


void instantiate_seach_engine_and_run(const FwdSearchProblem& search_prob, const ProblemInfo::cptr& problemInfo, float timeout, const std::string& out_dir) {
	float timer = 0.0;
	std::cout << "Starting search with Relaxed Plan Heuristic and GBFS (time budget is " << timeout << " secs)..." << std::endl;
	aptk::search::bfs::AT_GBFS_SQ_SH< aptk::core::FwdSearchProblem, RelaxedHeuristic, BFS_Open_List > rp_bfs_engine( search_prob );
// 	aptk::search::bfs::AT_GBFS_SQ_SH< aptk::core::FwdSearchProblem, RelaxedMaxHeuristic, BFS_Open_List > rp_bfs_engine( search_prob );
	timer = do_search(rp_bfs_engine, problemInfo, timeout, out_dir);
	std::cout << "Search completed in " << timer << " secs" << std::endl;
}

void reportActionsInfo(const Problem& problem) {
	std::cout << "Action Information: " << std::endl;
	for (unsigned i = 0; i < problem.getNumActions(); ++i) {
		const auto action = problem.getAction(i);
		std::cout << "Action #" << i << ": " << *action << std::endl;
	}
}

void reportProblemStats(const Problem& problem) {
	
// 	const auto& st = problem.get_symbol_table();
// 	
// 	std::cout << "Number of object types: " << st.get_num_types() << std::endl;
	std::cout << "Number of objects: " << problem.getProblemInfo()->getNumObjects() << std::endl;
	std::cout << "Number of state variables: " << problem.getProblemInfo()->getNumVariables() << std::endl;
	
	std::cout << "Number of ground actions: " << problem.getNumActions() << std::endl;
	if (problem.getNumActions() > 1000) {
		std::cout << "WARNING: The number of ground actions (" << problem.getNumActions() <<
		") is too high for our current applicable action strategy to perform well." << std::endl;
	}
	
	std::cout << "Number of state constraints: " << problem.getConstraints().size() << std::endl;
	std::cout << "Number of goal constraints: " << problem.getGoalConstraints().size() << std::endl;
	
// 	reportActionsInfo(problem);
}



int main( int argc, char** argv ) {
	if (argc != 4) {
		std::cerr << "Wrong number of parameters\nUsage: " << argv[0] << " timeout data_dir out_dir" << std::endl;
		return -1;
	}
	
	float timeout =  atof(argv[1]);
	std::string data_dir(argv[2]);
	std::string out_dir(argv[3]);
	
	ProblemInfo::cptr problemInfo(new ProblemInfo(data_dir));
	
	// Instantiate the problem
	std::cout << "Generating the problem... " << std::endl;
	Problem problem;
	aptk::core::solver::generate(data_dir, problem);
	aptk::core::FwdSearchProblem search_prob(problem);
	std::cout << "Done. " << std::endl;
	
	problem.setProblemInfo(problemInfo);
	Problem::setCurrentProblem(problem);
	
	reportProblemStats(problem);
	
	// Instantiate the engine
	instantiate_seach_engine_and_run(search_prob, problemInfo, timeout, out_dir);
	return 0;
}
