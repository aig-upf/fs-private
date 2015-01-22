
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include <fs0_types.hxx>
#include <state.hxx>

namespace fs0 {
class Problem;

class Loader
{
public:
	
	//! The type of an action factory
	typedef std::function<Action::cptr (
			const std::string&,
			const ObjectIdxVector&,
			const ObjectIdxVector&,
			const std::vector<VariableIdxVector>&,
			const std::vector<VariableIdxVector>&,
			const std::vector<VariableIdxVector>&
			)> ActionFactoryType;
	
	typedef std::function<ScopedConstraint::vcptr (const std::vector<VariableIdxVector>&)> GoalFactoryType;
	
	
	/**
	 * 
	 */
	static void loadProblem(const std::string& dir, ActionFactoryType actionFactory, GoalFactoryType goalFactory, Problem& problem);
	
	/**
	 * Loads a state specification for a given text file.
	 * The specification basically consists on an assignation of values to all the state variables.
	 */
	static const State::cptr loadStateFromFile(const std::string& filename);
	
	/**
	 * Instantiates all the grounded actions from the information stored in the given text files and by calling
	 * the specified function, which is generated dinamically and performs the actual instantiation.
	 */
	static void loadGroundedActions(const std::string& filename, ActionFactoryType actionFactory, Problem& problem);
	
	static void generateGoalConstraints(const std::string& filename, GoalFactoryType goalFactory, Problem& problem);
	
	//! Loads a set of state constraints from the given file
	static void loadConstraints(const std::string& filename, Problem& problem);
	
protected:
	template<typename T>
	static std::vector<T> parseNumberList(const std::string& input, const std::string sep = ",");
	
	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const std::string& input, const std::string sep = "/");
};

} // namespaces

