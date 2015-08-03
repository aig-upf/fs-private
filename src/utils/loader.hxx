
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include <fs0_types.hxx>
#include <state.hxx>

#include <lib/rapidjson/document.h>
#include <actions/action_schema.hxx>

namespace fs0 {

class BaseComponentFactory;
class Problem;

class Loader
{
public:
	
	//!
	static void loadProblem(const rapidjson::Document& data, const BaseComponentFactory& factory, Problem& problem);
	
	static rapidjson::Document loadJSONObject(const std::string& filename);
	
protected:
	
	typedef std::map<std::string, ActionSchema::cptr> SchemaIndex;
	
	 //! Loads a state specification for a given text file.
	 //! The specification basically consists on an assignation of values to all the state variables.
	static const State::cptr loadState(const rapidjson::Value& data);
	
	 //! Instantiates all the grounded actions from the information stored in the given text files and by calling
	 //! the specified function, which is generated dinamically and performs the actual instantiation.
	static void loadGroundedActions(const rapidjson::Value& data, const BaseComponentFactory& factory, const SchemaIndex& schema_index, Problem& problem);
	
	static SchemaIndex loadActionSchemata(const rapidjson::Value& data, Problem& problem);
	
	static void loadFunctions(const BaseComponentFactory& factory, Problem& problem, ProblemInfo& info);
	
	static ActionSchema::cptr loadActionSchema(const rapidjson::Value& data, const ProblemInfo& info);
	
	//! Load a set of already-grounded atomic formulae
	static std::vector<AtomicFormula::cptr> loadGroundedConditions(const rapidjson::Value& data, Problem& problem);
	
	// Conversion to a C++ vector of values.
	template<typename T>
	static std::vector<T> parseNumberList(const rapidjson::Value& data);
	static std::vector<std::string> parseStringList(const rapidjson::Value& data);
	
	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const rapidjson::Value& data);
};

} // namespaces

