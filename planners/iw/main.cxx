
#include <iostream>
#include <fstream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <aptk2/search/algorithms/best_first_search.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/tools/resources_control.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list_test.hxx>

#include <utils/logging.hxx>
#include <utils/loader.hxx>
#include <heuristics/null_heuristic.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <heuristics/hmax.hxx>
#include <heuristics/novelty_from_preconditions.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <utils/printers.hxx>
#include <utils/config.hxx>
#include <problem_info.hxx>
#include <action_manager.hxx>
#include <fwd_search_prob.hxx>

#include <components.hxx>  // This will dinamically point to the right generated file

using namespace fs0;

// MRJ: Now we define the heuristics
typedef		RelaxedPlanHeuristic<FwdSearchProblem> RelaxedHeuristic;


class NoveltyEvaluator {
public:
	const	FwdSearchProblem&							_problem;
	std::vector< NoveltyFromPreconditions >						_novelty_heuristic;
	unsigned									_max_novelty;

	NoveltyEvaluator( const FwdSearchProblem& problem ): 
		_problem( problem ), _max_novelty(0) {
		// MRJ: setups the novelty heuristic, this is all it needs to know
		_novelty_heuristic.resize( problem.getTask().numGoalConstraints() + 1 );
		std::cout << "# Novelty evaluators: " << _novelty_heuristic.size() << std::endl;
	}

	~NoveltyEvaluator() {
		for (unsigned j = 0; j < _novelty_heuristic.size(); j++)
			for ( unsigned k = 1; k <= novelty_bound(); k++ ) {
				std::cout << "# novelty(s)[#goals=" << j << "]=" << k << " : " << _novelty_heuristic[j].get_num_states(k) << std::endl;;
			}
	}

	void setup( int max_novelty, bool useStateVars, bool useGoal, bool useActions ) {
		// MRJ: setups the novelty heuristic, this is all it needs to know
		_max_novelty = max_novelty;
		for ( unsigned k = 0; k < _novelty_heuristic.size(); k++ ) {
			_novelty_heuristic[k].set_max_novelty( novelty_bound() );
			_novelty_heuristic[k].selectFeatures( _problem.getTask(), useStateVars, useGoal, useActions );
		}
	}

	unsigned evaluate_novelty( const GenericState& s ) {
		return _novelty_heuristic[evaluate_num_unsat_goals(s)].evaluate( s );
	}

	unsigned evaluate_num_unsat_goals( const GenericState& s ) {
		return _problem.getTask().numUnsatisfiedGoals(s);
	}

	unsigned novelty_bound() { return _max_novelty; }
	
	//! Returns false iff we want to prune this node during the search
	bool accept(const State& s) {
	  auto novelty = evaluate_novelty(s);
	  //std::cout << "Novelty value : " << novelty << " of state: " << s << std::endl;
	  return novelty <= novelty_bound();
	}
};



template <typename State>
class FS0_Node {
public:
	State								state;
	Action::IdType							action;
	std::shared_ptr<FS0_Node<State> >				parent;

public:
	// Kill default constructors
	explicit FS0_Node();
	
	//! Constructor for initial nodes, copies state
	FS0_Node( const State& s )
		: state( s ), action( Action::invalid_action_id ), parent( nullptr )
	{}

	//! Constructor for successor states, doesn't copy the state
	FS0_Node( State&& _state, Action::IdType _action, std::shared_ptr< FS0_Node<State> > _parent ) :
		state(_state) {
		action = _action;
		parent = _parent;
	}

	virtual ~FS0_Node() {}

	bool	has_parent() const { return parent != nullptr; }

	void	print( std::ostream& os ) const {
		os << "{@ = " << this << ", s = " << state << ", parent = " << parent << "}";
	}

	bool   	operator==( const FS0_Node<State>& o ) const { return state == o.state; }

	size_t hash() const { return state.hash(); }
};

// MRJ: We start defining the type of nodes for our planner
typedef FS0_Node<fs0::State> Search_Node;
typedef std::vector<Action::IdType> Plan;

bool checkPlanCorrect(const Plan& plan) {
	auto problem = Problem::getCurrentProblem();
	std::vector<unsigned> p;
	for (const auto& elem:plan) p.push_back((unsigned) elem);
	return ActionManager::checkPlanSuccessful(*problem, p, *(problem->getInitialState()));
}

template <typename Search_Engine>
float do_search( Search_Engine& engine, const ProblemInfo& problemInfo, const std::string& out_dir) {

	std::ofstream out(out_dir + "/searchlog.out");
	std::ofstream plan_out(out_dir + "/first.plan");
	std::ofstream json_out( out_dir + "/search.json" );

	std::cout << "Writing results to " << out_dir + "/searchlog.out" << std::endl;

	Plan plan;
	float t0 = aptk::time_used();
	bool solved = engine.solve_model( plan );
	float total_time = aptk::time_used() - t0;	



	bool valid = checkPlanCorrect(plan);
	if ( solved ) {
		PlanPrinter::printPlan(plan, problemInfo, out);
		PlanPrinter::printPlan(plan, problemInfo, plan_out);
	
	}

	out << "Domain: " << problemInfo.getDomainName() << std::endl;
	out << "Total time: " << total_time << std::endl;
	out << "Nodes generated during search: " << engine.generated << std::endl;
	out << "Nodes expanded during search: " << engine.expanded << std::endl;

	std::string eval_speed = (total_time > 0) ? std::to_string((float) engine.generated / total_time) : "-";
	out << "Heuristic evaluations per second: " <<  eval_speed << std::endl;

	out.close();
	plan_out.close();

	json_out << "{" << std::endl;
	json_out << "\t\"search_time\" : " << total_time << "," << std::endl;
	json_out << "\t\"generated\" : " << engine.generated << "," << std::endl;
	json_out << "\t\"expanded\" : " << engine.expanded << "," << std::endl;
	json_out << "\t\"eval_per_second\" : " << eval_speed << "," << std::endl;
	json_out << "\t\"solved\" : " << ( solved ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"valid\" : " << ( valid ? "true" : "false" ) << "," << std::endl;
	json_out << "\t\"num_goals\" : " << engine.model.getTask().numGoalConstraints() << "," << std::endl;
	json_out << "\t\"novelty_histogram\" : {" << std::endl;
	/*for ( unsigned j = 0; j < engine.heuristic_function._novelty_heuristic.size(); j++ ) {
		json_out << "\t\t\"" << j << "\": {" << std::endl;
		for ( unsigned i = 0; i <= engine.heuristic_function.novelty_bound(); i++ ) {
			json_out << "\t\t\t\t\"" << i << "\" : " << engine.heuristic_function._novelty_heuristic[j].get_num_states(i);
			if ( i < engine.heuristic_function.novelty_bound() ) json_out << ", ";
			json_out << std::endl;
		}
		json_out << "\t\t}";
		if ( j < engine.heuristic_function._novelty_heuristic.size() - 1)
			json_out << ",";
		json_out << std::endl;
	}*/
	json_out << "\t}," << std::endl;
	json_out << "\t\"plan\" : ";
	if ( solved )
		PlanPrinter::printPlanJSON( plan, problemInfo, json_out);
	else
		json_out << "null";
	json_out << std::endl;
	json_out <<  "}" << std::endl;

	json_out.close();

	return total_time;
}

typedef aptk::StlUnsortedFIFOTest<Search_Node, NoveltyEvaluator> FS0OpenList;

void instantiate_search_engine_and_run(
	const FwdSearchProblem& search_prob, const ProblemInfo& problemInfo, int max_novelty,
	bool useStateVars, bool useGoal, bool useActions,	int timeout, const std::string& out_dir) {
	float timer = 0.0;
	std::cout << "Starting search with Relaxed Plan/Novelty Heuristic greedy best first search (time budget is " << timeout << " secs)..." << std::endl;
	
// 	if (true) {
// 	  aptk::StlBestFirstSearch< Search_Node, NoveltyEvaluator, FwdSearchProblem > rp_bfs_engine( search_prob );
// 	  rp_bfs_engine.heuristic_function.setup(max_novelty, useStateVars, useGoal, useActions );
// 	} else {
	    auto evaluator = std::make_shared<NoveltyEvaluator>(search_prob);
	    evaluator->setup(max_novelty, useStateVars, useGoal, useActions );
	    FS0OpenList open_list(evaluator);
	    aptk::StlBreadthFirstSearch<Search_Node, FwdSearchProblem, FS0OpenList> brfs_iw_engine(search_prob, open_list);
	    
// 	}
	std::cout << "Heuristic options:" << std::endl;
	std::cout << "\tMax novelty: " << max_novelty << std::endl;
	std::cout << "\tUsing state vars as features: " << ( useStateVars ? "yes" : "no ") << std::endl;
	std::cout << "\tUsing goal as feature: " << ( useGoal ? "yes" : "no ") << std::endl;
	std::cout << "\tUsing actions as features: " << ( useActions ? "yes" : "no ") << std::endl;
	
	timer = do_search(brfs_iw_engine, problemInfo, out_dir);
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

// 	std::cout << "Number of object types: " << st.get_num_types() << std::endl;
	std::cout << "Number of objects: " << problem.getProblemInfo().getNumObjects() << std::endl;
	std::cout << "Number of state variables: " << problem.getProblemInfo().getNumVariables() << std::endl;

	std::cout << "Number of ground actions: " << problem.getNumActions() << std::endl;
	if (problem.getNumActions() > 1000) {
		std::cout << "WARNING: The number of ground actions (" << problem.getNumActions() <<
		") is too high for our current applicable action strategy to perform well." << std::endl;
	}

	std::cout << "Number of state constraints: " << problem.getConstraints().size() << std::endl;
	std::cout << "Number of goal constraints: " << problem.getGoalConstraints().size() << std::endl;

// 	reportActionsInfo(problem);
}


namespace po = boost::program_options;

int parse_options(int argc, char** argv, int& timeout, std::string& data_dir, std::string& out_dir, 
		  int& max_novelty, bool& use_state_vars, bool& use_goal, bool& use_actions ) {
  
	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "Display this help message")
		("timeout,t", po::value<int>()->default_value(10), "The timeout, in seconds.")
		("data", po::value<std::string>()->default_value("data"), "The directory where the input data is stored.")
		("out", po::value<std::string>()->default_value("."), "The directory where the results data is to be output.")
		("max_novelty,n", po::value<int>()->default_value(2), "Maximum novelty (defaults to 2)")
		("use_state_vars", po::value<bool>()->default_value(true), "Use state variables as features (default is true)")
		("use_goal", po::value<bool>()->default_value(true), "Use goal satisfiability as a feature (default is true)")
		("use_actions", po::value<bool>()->default_value(false), "Use action preconditions satisfiability as a feature (default is false)");

	po::positional_options_description pos;
	pos.add("timeout", 1)
	   .add("data", 1)
	   .add("out", 1)
	   .add("max_novelty", 1);

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
	max_novelty = vm["max_novelty"].as<int>();
	use_state_vars = vm["use_state_vars"].as<bool>();
	use_goal = vm["use_goal"].as<bool>();
	use_actions = vm["use_actions"].as<bool>();
	return 0;
}


int main(int argc, char** argv) {

	int timeout, max_novelty; std::string data_dir; std::string out_dir;
	bool use_state_vars, use_goal, use_actions;
	int res = parse_options(argc, argv, timeout, data_dir, out_dir, max_novelty, use_state_vars, use_goal, use_actions);
	if (res != 0) 
		return res;
	

	Logger::init("./logs");
	Config::init("config.json");

	FINFO("main", "Planner configuration: " << std::endl << Config::instance());
	FINFO("main", "Generating the problem (" << data_dir << ")... ");


	std::cout << "Generating the problem (" << data_dir << ")... " << std::endl;
	auto data = Loader::loadJSONObject(data_dir + "/problem.json");
	Problem problem(data);

	std::cout << "Calling generate()" << std::endl;
	generate(data, data_dir, problem);

	std::cout << "Setting current problem to problem" << std::endl;
	Problem::setCurrentProblem(problem);

	reportProblemStats(problem);
	problem.analyzeVariablesRelevance();

	std::cout << "Creating Search_Problem instance" << std::endl;
	FwdSearchProblem search_prob(problem);


	std::cout << "Done!" << std::endl;

	// Instantiate the engine
	instantiate_search_engine_and_run(search_prob, problem.getProblemInfo(), max_novelty, use_state_vars, use_goal, use_actions, timeout, out_dir);
	return 0;
}
