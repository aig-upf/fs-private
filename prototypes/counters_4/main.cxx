
#include <iostream>
#include <fstream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <aptk2/search/algorithms/best_first_search.hxx>
#include <aptk2/tools/resources_control.hxx>

#include <heuristics/null_heuristic.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <heuristics/hmax.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <utils/printers.hxx>
#include <problem_info.hxx>
#include <action_manager.hxx>
#include <fwd_search_prob.hxx>
#include <constraints/gecode/constraint_translation.hxx>

#include <components.hxx>  // This will dinamically point to the right generated file

using namespace fs0;

// MRJ: Now we define the heuristics
typedef		RelaxedPlanHeuristic<FwdSearchProblem> RelaxedHeuristic;
typedef		HMaxHeuristic<FwdSearchProblem> RelaxedMaxHeuristic;


template <typename State>
class FS0_Node {
public:

	typedef State State_Type;


	// Kill default constructors
	explicit FS0_Node();
	//! Constructor for initial nodes, copies state
	FS0_Node( const State& s )
		: state( s ), action( Action::invalid_action_id ), parent( nullptr ), g(0), is_dead_end(false) {
	}

	//! Constructor for successor states, doesn't copy the state
	FS0_Node( State&& _state, Action::IdType _action, std::shared_ptr< FS0_Node<State> > _parent ) :
		state(_state) {
		action = _action;
		parent = _parent;
		g = _parent->g + 1;
		is_dead_end = false;
	}

	virtual ~FS0_Node() {
	}

	bool	has_parent() const { return parent != nullptr; }

	void			print( std::ostream& os ) const {
		os << "{@ = " << this << ", s = " << state << ", h = " << h << ", parent = " << parent << "}";
	}

	bool   	operator==( const FS0_Node<State>& o ) const {
		return state == o.state;
	}

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	void	evaluate_with( Heuristic& heuristic ) {
		h = heuristic.evaluate( state );
	}

	size_t                  hash() const { return state.hash(); }

	// MRJ: With this we implement Greedy Best First search
	bool	operator>( const FS0_Node<State>& other ) const {
		return h > other.h;
	}

	bool	dead_end() const {
		return is_dead_end;
	}

public:

	State															state;
	Action::IdType										action;
	std::shared_ptr<FS0_Node<State> >	parent;
	unsigned													h;
	unsigned													g;
	bool															is_dead_end;
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
		Printers::printPlan(plan, problemInfo, out);
		Printers::printPlan(plan, problemInfo, plan_out);
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
		Printers::printPlanJSON( plan, problemInfo, json_out);
	else
		json_out << "null";
	json_out << std::endl;
	json_out << "}" << std::endl;

	json_out.close();

	return total_time;
}


void instantiate_seach_engine_and_run(const FwdSearchProblem& search_prob, const ProblemInfo& problemInfo, int timeout, const std::string& out_dir) {
	float timer = 0.0;
	std::cout << "Starting search with Relaxed Plan Heuristic and GBFS (time budget is " << timeout << " secs)..." << std::endl;
	aptk::StlBestFirstSearch< Search_Node, RelaxedHeuristic, FwdSearchProblem > rp_bfs_engine( search_prob );
	timer = do_search(rp_bfs_engine, problemInfo, out_dir);
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


	gecode::registerTranslators();

	std::cout << "Generating the problem (" << data_dir << ")... " << std::endl;
	Problem problem(data_dir);

	std::cout << "Calling generate()" << std::endl;
	generate(data_dir, problem);

	std::cout << "Setting current problem to problem" << std::endl;
	Problem::setCurrentProblem(problem);

	std::cout << "Creating Search_Problem instance" << std::endl;
	FwdSearchProblem search_prob(problem);


	std::cout << "Done!" << std::endl;
	reportProblemStats(problem);

	// Instantiate the engine
	instantiate_seach_engine_and_run(search_prob, problem.getProblemInfo(), timeout, out_dir);
	return 0;
}
