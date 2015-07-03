
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include <fs0_types.hxx>
#include <state.hxx>
#include <actions.hxx>
#include "lib/rapidjson/document.h"

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
			const VariableIdxVector&
			)> ActionFactoryType;
	
	typedef std::function<ScopedConstraint::vcptr (const std::vector<VariableIdxVector>&)> GoalFactoryType;
	
	typedef std::function<ScopedConstraint::cptr (const std::string& classname, const ObjectIdxVector&, const VariableIdxVector&)> ConstraintFactoryType;
	
	
	/**
	 * 
	 */
	static void loadProblem(const rapidjson::Document& data, ActionFactoryType actionFactory, ConstraintFactoryType constraintFactory, GoalFactoryType goalFactory, Problem& problem);
	
	/**
	 * Loads a state specification for a given text file.
	 * The specification basically consists on an assignation of values to all the state variables.
	 */
	static const State::cptr loadState(const rapidjson::Value& data);
	
	/**
	 * Instantiates all the grounded actions from the information stored in the given text files and by calling
	 * the specified function, which is generated dinamically and performs the actual instantiation.
	 */
	static void loadGroundedActions(const rapidjson::Value& data, ActionFactoryType actionFactory, Problem& problem);
	
	static void generateGoalConstraints(const rapidjson::Value& data, GoalFactoryType goalFactory, Problem& problem);
	
	//! Loads a set of state constraints from the given file
	static void loadConstraints(const rapidjson::Value& data, ConstraintFactoryType constraintFactory, Problem& problem);
	
	static rapidjson::Document loadJSONObject(const std::string& filename);
	
protected:
	// Conversion to a C++ vector of values.
	template<typename T>
	static std::vector<T> parseNumberList(const rapidjson::Value& data);
	
	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const rapidjson::Value& data);
};

} // namespaces

