
#pragma once

#include <vector>
#include <lib/rapidjson/document.h>

#include <utils/lexical_cast.hxx>


namespace fs0 { namespace language { namespace fstrips { class Formula; class Axiom; class Metric; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class BaseComponentFactory;
class ProblemInfo;
class State;
class StateAtomIndexer;
class ActionData;
class GroundAction;
class Problem;

class Loader {
public:
	//! Load and set the singleton problem instance
	static Problem* loadProblem(const rapidjson::Document& data);

	//! Load and set the singleton problemInfo instance
	static ProblemInfo& loadProblemInfo(const rapidjson::Document& data, const std::string& data_dir, const BaseComponentFactory& factory);

	static rapidjson::Document loadJSONObject(const std::string& filename);

	// Conversion to a C++ vector of values.
	template<typename T>
	static std::vector<T> parseNumberList(const rapidjson::Value& data);
	static std::vector<std::string> parseStringList(const rapidjson::Value& data);

protected:

	 //! Loads a state specification for a given text file.
	 //! The specification basically consists on an assignation of values to all the state variables.
	static State* loadState(const StateAtomIndexer& indexer, const rapidjson::Value& data);

	//! Load the data related to the problem functions and predicates into the info object
	static void loadFunctions(const BaseComponentFactory& factory, ProblemInfo& info);

	static std::vector<const fs::Axiom*> loadAxioms(const rapidjson::Value& data, const ProblemInfo& info);

	static std::vector<const ActionData*> loadAllActionData(const rapidjson::Value& data, const ProblemInfo& info, bool load_effects);

	static const ActionData* loadActionData(const rapidjson::Value& data, unsigned id, const ProblemInfo& info, bool load_effects);

	//! Load a formula and process it
	static const fs::Formula* loadGroundedFormula(const rapidjson::Value& data, const ProblemInfo& info);
    static std::vector<const fs::Axiom*> loadNamedStateConstraints(const rapidjson::Value& data, const ProblemInfo& info);

    //! Load the metric
    static const fs::Metric* loadMetric( const rapidjson::Value& data, const ProblemInfo& info );

	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const rapidjson::Value& data);
};

template<typename T>
std::vector<T>
Loader::parseNumberList(const rapidjson::Value& data) {
	std::vector<T> output;
	for (unsigned i = 0; i < data.Size(); ++i) {
		output.push_back(boost::lexical_cast<T>(data[i].GetInt()));
	}
	return output;
}

} // namespaces
