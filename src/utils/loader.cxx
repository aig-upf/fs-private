
#include <cassert>
#include <memory>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <constraints/constraint_factory.hxx>
#include <actions.hxx>

#include <iostream>

namespace fs0 {

void Loader::loadProblem(const rapidjson::Document& data, ActionFactoryType actionFactory, ConstraintFactoryType constraintFactory, GoalFactoryType goalFactory, Problem& problem) {
	/* Define the actions */
	std::cout << "\tDefining actions..." << std::endl;
	loadGroundedActions(data["actions"], actionFactory, problem);

	/* Define the initial state */
	std::cout << "\tDefining initial state..." << std::endl;
	problem.setInitialState(loadState(data["init"]));

	/* Load the state and goal constraints */
	std::cout << "\tDefining state and goal constraints..." << std::endl;
	loadConstraints(data["constraints"], constraintFactory, problem);

	/* Generate goal constraints from the goal evaluator */
	std::cout << "\tGenerating goal constraints..." << std::endl;
	generateGoalConstraints(data["goal"], goalFactory, problem);
}


const State::cptr Loader::loadState(const rapidjson::Value& data) {
	// The state is an array of two-sized arrays [x,v], representing atoms x=v
	unsigned numAtoms = data["variables"].GetInt();
	Atom::vctr facts;
	for (unsigned i = 0; i < data["atoms"].Size(); ++i) {
		const rapidjson::Value& node = data["atoms"][i];
		facts.push_back(Atom(node[0].GetInt(), node[1].GetInt()));
	}
	return std::make_shared<State>(numAtoms, facts);
}

void Loader::loadGroundedActions(const rapidjson::Value& data, ActionFactoryType actionFactory, Problem& problem) {
	assert(problem.getNumActions() == 0);
	
	for (unsigned i = 0; i < data.Size(); ++i) {
		const rapidjson::Value& node = data[i];
		
// 		# Format: a number of elements defining the action:
// 		# (0) Action ID (index)
// 		# (1) Action name
// 		# (2) classname
// 		# (3) binding
// 		# (4) derived objects
// 		# (5) applicability relevant vars
// 		# (6) effect relevant vars
// 		# (7) effect affected vars
		
		
		// We ignore the grounded name for the moment being.
		const std::string& actionClassname = node[2].GetString();
		ObjectIdxVector binding = parseNumberList<int>(node[3]);
		ObjectIdxVector derived = parseNumberList<int>(node[4]);
		std::vector<VariableIdxVector> appRelevantVars = parseDoubleNumberList<unsigned>(node[5]);
		std::vector<VariableIdxVector> effRelevantVars = parseDoubleNumberList<unsigned>(node[6]);
		std::vector<VariableIdxVector> effAffectedVars = parseDoubleNumberList<unsigned>(node[7]);
		
		problem.addAction(actionFactory(actionClassname, binding, derived, appRelevantVars, effRelevantVars, effAffectedVars));
	}
}

void Loader::generateGoalConstraints(const rapidjson::Value& data, GoalFactoryType goalFactory, Problem& problem) {
	// We have one single line with the variables indexes relevant to the goal procedures.
	std::vector<VariableIdxVector> appRelevantVars = parseDoubleNumberList<unsigned>(data);
	ScopedConstraint::vcptr goal_constraints = goalFactory(appRelevantVars);
	
	for (const auto& ctr:goal_constraints) {
		problem.registerGoalConstraint(ctr);
	}
}

void Loader::loadConstraints(const rapidjson::Value& data, ConstraintFactoryType constraintFactory, Problem& problem) {

	for (unsigned i = 0; i < data.Size(); ++i) {
		const rapidjson::Value& node = data[i];
		
		// Each node contains 4 elements
		// (0) The constraint description
		// (1) The constraint name
		// (2) The constraint parameters
		// (3) The variables upon which the constraint is enforced
		assert(node.Size()==4);

		// We ignore the grounded name for the moment being.
		const std::string& name = node[1].GetString();
		ObjectIdxVector parameters = parseNumberList<int>(node[2]);
		VariableIdxVector variables = parseNumberList<unsigned>(node[3]);
		
		problem.registerConstraint(constraintFactory(name, parameters, variables));
	}
}

rapidjson::Document Loader::loadJSONObject(const std::string& filename) {
	// Load and parse the JSON data file.
	std::ifstream in(filename);
	if ( in.fail() ) {
		std::cerr << "Could not open filename '" << filename << "'" << std::endl;
		std::exit(1);
	}
	std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	rapidjson::Document data;
	data.Parse(str.c_str());
	return data;
}


template<typename T>
std::vector<T> Loader::parseNumberList(const rapidjson::Value& data) {
	std::vector<T> output;
	for (unsigned i = 0; i < data.Size(); ++i) {
		output.push_back(boost::lexical_cast<T>(data[i].GetInt()));
	}
	return output;
}


template<typename T>
std::vector<std::vector<T>> Loader::parseDoubleNumberList(const rapidjson::Value& data) {
	std::vector<std::vector<T>> output;
	assert(data.IsArray());
	if (data.Size() == 0) {
		output.push_back(std::vector<T>());
	} else {
		for (unsigned i = 0; i < data.Size(); ++i) {
			output.push_back(parseNumberList<T>(data[i]));
		}
	}
	return output;
}


} // namespaces
