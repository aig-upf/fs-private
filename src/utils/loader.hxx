
#pragma once

#include <fs0_types.hxx>
#include <lib/rapidjson/document.h>

namespace fs0 {

class BaseComponentFactory;
class Problem;
class State;

class Loader {
public:
	//! Load and set the singleton problem instance
	static void loadProblem(const rapidjson::Document& data);
	
	//! Load and set the singleton problemInfo instance
	static void loadProblemInfo(const rapidjson::Document& data, const BaseComponentFactory& factory);
	
	static rapidjson::Document loadJSONObject(const std::string& filename);
	
protected:
	
	 //! Loads a state specification for a given text file.
	 //! The specification basically consists on an assignation of values to all the state variables.
	static State* loadState(const rapidjson::Value& data);
	
	static std::vector<const ActionSchema*> loadActionSchemata(const rapidjson::Value& data, const ProblemInfo& info);
	
	//! Load the data related to the problem functions and predicates into the info object
	static void loadFunctions(const BaseComponentFactory& factory, ProblemInfo& info);
	
	static const ActionSchema* loadActionSchema(const rapidjson::Value& data, unsigned id, const ProblemInfo& info);
	
	//! Load a formula and process it
	static const fs::Formula* loadGroundedFormula(const rapidjson::Value& data, const ProblemInfo& info);
	
	// Conversion to a C++ vector of values.
	template<typename T>
	static std::vector<T> parseNumberList(const rapidjson::Value& data);
	static std::vector<std::string> parseStringList(const rapidjson::Value& data);
	
	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const rapidjson::Value& data);
};

} // namespaces

