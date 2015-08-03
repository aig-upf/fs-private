
#include <cassert>
#include <memory>
#include <boost/lexical_cast.hpp>

#include <problem.hxx>
#include <utils/loader.hxx>
#include <actions/ground_action.hxx>
#include <component_factory.hxx>

#include <iostream>

#include <languages/fstrips/loader.hxx>
#include <heuristics/rpg/action_manager_factory.hxx>
#include <actions/ground_action.hxx>

#include <component_factory.hxx>
#include <utils/logging.hxx>
#include <utils/printers/binding.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

void Loader::loadProblem(const rapidjson::Document& data, const BaseComponentFactory& factory, Problem& problem) {
	
	// Load and set the ProblemInfo data structure
	auto info = new ProblemInfo(data);
	problem.setProblemInfo(info);
	
	//! Load the actual static functions
	loadFunctions(factory, problem, *info);
	
	/* Define the actions */
	std::cout << "\tDefining actions..." << std::endl;
	auto schema_index = loadActionSchemata(data["action_schemata"], problem);
	
	loadGroundedActions(data["actions"], factory, schema_index, problem);
	
	/* Define the initial state */
	std::cout << "\tDefining initial state..." << std::endl;
	problem.setInitialState(loadState(data["init"]));

	/* Load the state and goal constraints */
	std::cout << "\tDefining state and goal constraints..." << std::endl;
	assert(problem.getStateConstraints().empty());
	problem.setStateConstraints(loadGroundedConditions(data["state_constraints"], problem));

	/* Generate goal constraints from the goal evaluator */
	std::cout << "\tGenerating goal constraints..." << std::endl;
	assert(problem.getGoalConditions().empty());
	problem.setGoalConditions(loadGroundedConditions(data["goal"], problem));
}

void Loader::loadFunctions(const BaseComponentFactory& factory, Problem& problem, ProblemInfo& info) {
	for (auto elem:factory.instantiateFunctions()) {
		info.setFunction(info.getFunctionId(elem.first), elem.second);
	}
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

void Loader::loadGroundedActions(const rapidjson::Value& data, const BaseComponentFactory& factory, const SchemaIndex& schema_index, Problem& problem) {
	assert(problem.getGroundActions().empty());
	
	for (unsigned i = 0; i < data.Size(); ++i) {
		const rapidjson::Value& node = data[i];
		
// 		# Format: a number of elements defining the action:
// 		# (0) Action ID (index)
// 		# (1) Action name
// 		# (2) classname
// 		# (3) binding
		
		
		// We ignore the grounded name for the moment being.
		const std::string& actionClassname = node[2].GetString();
		ObjectIdxVector binding = parseNumberList<int>(node[3]);
		
		// This is no longer necessary, we perform the grounding in the engine
// 		problem.addAction(factory.instantiateAction(actionClassname, binding, derived, appRelevantVars, effRelevantVars, effAffectedVars));
		
		
		// XXX - With GroundActions
		ActionSchema::cptr schema = schema_index.at(actionClassname);
		
		FDEBUG("grounding", "Processing with binding " << print::binding(binding, schema->getSignature()) << " action schema...\n" << *schema);
		GroundAction* ground = schema->process(binding, problem.getProblemInfo());
		if (ground) {
			FDEBUG("grounding", "Generated grounded action:\n" << *ground);
			problem.addGroundAction(ground);
		} else {
			FDEBUG("grounding", "Grounded action is statically non-applicable");
		}
	}
}

Loader::SchemaIndex Loader::loadActionSchemata(const rapidjson::Value& data, Problem& problem) {
	assert(problem.getActionSchemata().empty());
	
	SchemaIndex index;
	for (unsigned i = 0; i < data.Size(); ++i) {
		ActionSchema::cptr schema = loadActionSchema(data[i], problem.getProblemInfo());
 		problem.addActionSchema(schema);
		index.insert(std::make_pair(schema->getClassname(), schema));
	}
	return index;
}

ActionSchema::cptr Loader::loadActionSchema(const rapidjson::Value& node, const ProblemInfo& info) {
	const std::string& name = node["name"].GetString();
	const std::string& classname = node["classname"].GetString();
	const std::vector<TypeIdx> signature = parseNumberList<unsigned>(node["signature"]);
	const std::vector<std::string> parameters = parseStringList(node["parameters"]);
	
	const std::vector<AtomicFormulaSchema::cptr> conditions = fs::Loader::parseAtomicFormulaList(node["conditions"], info);
	const std::vector<ActionEffectSchema::cptr> effects = fs::Loader::parseAtomicEffectList(node["effects"], info);
	
	return new ActionSchema(name, classname, signature, parameters, conditions, effects);
}

std::vector<AtomicFormula::cptr> Loader::loadGroundedConditions(const rapidjson::Value& data, Problem& problem) {
	std::vector<AtomicFormula::cptr> processed;
	std::vector<AtomicFormulaSchema::cptr> conditions = fs::Loader::parseAtomicFormulaList(data["conditions"], problem.getProblemInfo());
	for (const AtomicFormulaSchema::cptr condition:conditions) {
		processed.push_back(condition->process({}, problem.getProblemInfo())); // The conditions are by definition already grounded, thus we need no binding
		delete condition;
	}
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
