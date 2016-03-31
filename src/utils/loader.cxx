
#include <memory>
#include <boost/lexical_cast.hpp>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <actions/action_schema.hxx>
#include <actions/ground_action.hxx>
#include <actions/grounding.hxx>
#include <component_factory.hxx>
#include <languages/fstrips/loader.hxx>
#include <utils/logging.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/registry.hxx>
#include <utils/printers/registry.hxx>
#include <asp/lp_handler.hxx>
#include "config.hxx"
#include "tuple_index.hxx"


namespace fs = fs0::language::fstrips;

namespace fs0 {

void Loader::loadProblem(const rapidjson::Document& data, asp::LPHandler* lp_handler) {
	const ProblemInfo& info = Problem::getInfo();
	
	FINFO("main", "Loading initial state...");
	auto init = loadState(data["init"]);
	
	FINFO("main", "Loading action schemata...");
	auto schemata = loadActionSchemata(data["action_schemata"], info);
	
	FINFO("main", "Loading goal formula...");
	auto goal = loadGroundedFormula(data["goal"], info);
	
	FINFO("main", "Loading state constraints...");
	auto sc = loadGroundedFormula(data["state_constraints"], info);
	
	//! Set the singleton global instance
	Problem* problem = new Problem(init, schemata, goal, sc, TupleIndex(info));
	Problem::setInstance(std::unique_ptr<Problem>(problem));
	problem->setLPHandler(lp_handler);
	
	FINFO("components", "Bootstrapping problem with following external component repository\n" << print::logical_registry(LogicalComponentRegistry::instance()));

	// Ground the actions
	if (!Config::instance().doLiftedPlanning()) {
		problem->setGroundActions(ActionGrounder::ground(problem->getActionSchemata(), info));
	}
}

void Loader::loadFunctions(const BaseComponentFactory& factory, ProblemInfo& info) {
	for (auto elem:factory.instantiateFunctions()) {
		info.setFunction(info.getSymbolId(elem.first), elem.second);
	}
}

void Loader::loadProblemInfo(const rapidjson::Document& data, const BaseComponentFactory& factory) {
	// Load and set the ProblemInfo data structure
	auto info = new ProblemInfo(data);
	loadFunctions(factory, *info);
	Problem::setInfo(info);
}

State* Loader::loadState(const rapidjson::Value& data) {
	// The state is an array of two-sized arrays [x,v], representing atoms x=v
	unsigned numAtoms = data["variables"].GetInt();
	Atom::vctr facts;
	for (unsigned i = 0; i < data["atoms"].Size(); ++i) {
		const rapidjson::Value& node = data["atoms"][i];
		facts.push_back(Atom(node[0].GetInt(), node[1].GetInt()));
	}
	return new State(numAtoms, facts);
}


std::vector<ActionSchema::cptr> Loader::loadActionSchemata(const rapidjson::Value& data, const ProblemInfo& info) {
	std::vector<ActionSchema::cptr> schemata;
	for (unsigned i = 0; i < data.Size(); ++i) {
 		schemata.push_back(loadActionSchema(data[i], i, info));
	}
	return schemata;
}

ActionSchema::cptr Loader::loadActionSchema(const rapidjson::Value& node, unsigned id, const ProblemInfo& info) {
	const std::string& name = node["name"].GetString();
	const Signature signature = parseNumberList<unsigned>(node["signature"]);
	const std::vector<std::string> parameters = parseStringList(node["parameters"]);
	
	const fs::Formula::cptr precondition = fs::Loader::parseFormula(node["conditions"], info);
	const std::vector<fs::ActionEffect::cptr> effects = fs::Loader::parseEffectList(node["effects"], info);
	
	// We perform a first binding on the action schema so that state variables, etc. get consolidated, but the parameters remain the same
	// This is possibly not optimal, since for some configurations we might be duplicating efforts, but ATM we are happy with it
	auto schema = new ActionSchema(id, name, signature, parameters, precondition, effects);
	auto processed = schema->process(info);
	delete schema;
	return processed;
}

fs::Formula::cptr Loader::loadGroundedFormula(const rapidjson::Value& data, const ProblemInfo& info) {
	const fs::Formula::cptr unprocessed = fs::Loader::parseFormula(data["conditions"], info);
	// The conditions are by definition already grounded, and hence we need no binding, but we process the formula anyway
	// to detect tautologies, contradictions, etc., and to consolidate state variables
	auto processed = unprocessed->bind(Binding(), info);
	delete unprocessed;
	return processed;
}

rapidjson::Document Loader::loadJSONObject(const std::string& filename) {
	// Load and parse the JSON data file.
	std::ifstream in(filename);
	if (in.fail()) throw std::runtime_error("Could not open filename '" + filename + "'");
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

std::vector<std::string> Loader::parseStringList(const rapidjson::Value& data) {
	std::vector<std::string> output;
	for (unsigned i = 0; i < data.Size(); ++i) {
		output.push_back(data[i].GetString());
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
