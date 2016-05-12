
#pragma once

#include <vector>
#include <lib/rapidjson/document.h>

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace asp { class LPHandler; }}

namespace fs0 {

class BaseComponentFactory;
class ProblemInfo;
class State;
class ActionData;
class Problem;

class Loader {
public:
	//! Load and set the singleton problem instance
	static Problem* loadProblem(const rapidjson::Document& data, asp::LPHandler* lp_handler);
	
	//! Load and set the singleton problemInfo instance
	static const ProblemInfo& loadProblemInfo(const rapidjson::Document& data, const std::string& data_dir, const BaseComponentFactory& factory);
	
	static rapidjson::Document loadJSONObject(const std::string& filename);
	
protected:
	
	 //! Loads a state specification for a given text file.
	 //! The specification basically consists on an assignation of values to all the state variables.
	static State* loadState(const rapidjson::Value& data);

	//! Load the data related to the problem functions and predicates into the info object
	static void loadFunctions(const BaseComponentFactory& factory, const std::string& data_dir, ProblemInfo& info);
	
	static std::vector<const ActionData*> loadAllActionData(const rapidjson::Value& data, const ProblemInfo& info);
	
	static const ActionData* loadActionData(const rapidjson::Value& data, unsigned id, const ProblemInfo& info);
	
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
