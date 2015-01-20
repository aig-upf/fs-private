
#include <cassert>
#include <memory>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <constraints/factory.hxx>
#include <app_entity.hxx>

namespace fs0 {

void Loader::loadProblem(const std::string& dir, ActionFactoryType actionFactory, GoalFactoryType goalFactory, Problem& problem) {
	/* Define the actions */
	loadGroundedActions(dir + "/actions.data", actionFactory, problem);

	/* Define the initial state */
	problem.setInitialState(loadStateFromFile(dir + "/init.data"));

	/* Load the state and goal constraints */
	loadConstraints(dir + "/constraints.data", problem, false);
	loadConstraints(dir + "/goal-constraints.data", problem, true);

	/* Generate goal constraints from the goal evaluator */
	generateGoalConstraints(dir + "/goal.data", goalFactory, problem);
	
	problem.createConstraintManager();		
}
	
	
const State::cptr Loader::loadStateFromFile(const std::string& filename) {
	Fact::vctr facts;
	std::string str;
	std::ifstream in(filename);
	
	// Parse the total number of facts
	std::getline(in, str);
	unsigned numFacts = std::stoi(str);
	
	// Parse the initial facts themselves
	std::getline(in, str);
	typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
	Tokenizer tok(str);
	for (auto factStr:tok) {
		std::vector<std::string> strs;
		boost::split(strs, factStr, boost::is_any_of("="));
		facts.push_back(Fact(std::stoi(strs[0]), std::stoi(strs[1])));
	}
// 		std::cout << numFacts << std::endl;
// 		for (auto f: facts) std::cout << f << std::endl;
	return std::make_shared<State>(numFacts, facts);
}

void Loader::loadGroundedActions(const std::string& filename, ActionFactoryType actionFactory, Problem& problem) {
	std::string line;
	std::ifstream in(filename);
	
	// Parse the ground actions - each line is an action
	while (std::getline(in, line)) {
		// std::cout << line << std::endl;
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("#"));
		
		// strs contains 7 elements: 
		// # (0) Action name
		// # (1) classname
		// # (2) binding
		// # (3) derived objects
		// # (4) applicability relevant vars
		// # (5) effect relevant vars
		// # (6) effect affected vars
		assert(strs.size()==7);
		
		
		// We ignore the grounded name for the moment being.
		const std::string& actionClassname = strs[1];
		ObjectIdxVector binding = parseNumberList<int>(strs[2]);
		ObjectIdxVector derived = parseNumberList<int>(strs[3]);
		std::vector<VariableIdxVector> appRelevantVars = parseDoubleNumberList<unsigned>(strs[4]);
		std::vector<VariableIdxVector> effRelevantVars = parseDoubleNumberList<unsigned>(strs[5]);
		std::vector<VariableIdxVector> effAffectedVars = parseDoubleNumberList<unsigned>(strs[6]);

		problem.addAction(actionFactory(actionClassname, binding, derived, appRelevantVars, effRelevantVars, effAffectedVars));
	}
}

void Loader::generateGoalConstraints(const std::string& filename, GoalFactoryType goalFactory, Problem& problem) {
	std::string line;
	std::ifstream in(filename);
	
	// Parse the goal relevant variables - we simply have one line with the variables indexes.
	std::getline(in, line);
	
	std::vector<VariableIdxVector> appRelevantVars = parseDoubleNumberList<unsigned>(line);
	ScopedConstraint::vcptr goal_constraints = goalFactory(appRelevantVars);
	
	for (const auto& ctr:goal_constraints) {
		problem.registerGoalConstraint(ctr);
	}
}

void Loader::loadConstraints(const std::string& filename, Problem& problem, bool goal) {
	std::string line;
	std::ifstream in(filename);
	
	// Each line encodes a constraint.
	while (std::getline(in, line)) {
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("#"));
		
		// strs contains 3 elements: 
		// (0) The constraint description
		// (1) The constraint name
		// (2) The variables upon which the constraint is enforced
		assert(strs.size()==3);

		// We ignore the grounded name for the moment being.
		const std::string& name = strs[1];
		VariableIdxVector variables = parseNumberList<unsigned>(strs[2]);
		
		ProblemConstraint::cptr constraint = ConstraintFactory::create(name, variables);
		throw std::runtime_error("UNFINISHED");
		// TODO - REACTIVATE WITH THE NEW SCOPED CONSTRAINT LOGIC
// 		if (goal) {
// 			problem.registerGoalConstraint(constraint);
// 		} else {
// 			problem.registerConstraint(constraint);
// 		}
	}
}

template<typename T>
std::vector<T> Loader::parseNumberList(const std::string& input, const std::string sep) {
	if (input == "") return std::vector<T>();
	
	std::vector<T> output;
	
	std::vector<std::string> strs;
	boost::split(strs, input, boost::is_any_of(sep));

	for(const auto& str:strs) {
		output.push_back(boost::lexical_cast<T>(str));
	}
	return output;
}

template<typename T>
std::vector<std::vector<T>> Loader::parseDoubleNumberList(const std::string& input, const std::string sep) {
	std::vector<std::vector<T>> output;
	std::vector<std::string> strs;
	boost::split(strs, input, boost::is_any_of(sep));
	
	if (strs.size() == 0) {
		output.push_back(std::vector<T>());
	} else {
		for(const auto& str:strs) {
			output.push_back(parseNumberList<T>(str));
		}
	}
	return output;
}
} // namespaces
