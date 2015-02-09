
#include <problem_info.hxx>

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <atoms.hxx>

namespace fs0 {

// e.g. "int[-3..10]"
const boost::regex ProblemInfo::boundedIntRE("^int\\[(.*)\\.\\.(.*)\\]$");
	
ProblemInfo::ProblemInfo(const std::string& data_dir) {
	loadTypeIndex(data_dir + "/types.data"); // Order matters
	loadActionIndex(data_dir + "/action-index.data");
	loadObjectIndex(data_dir + "/objects.data");
	loadTypeObjects(data_dir + "/object-types.data");
	loadVariableIndex(data_dir + "/variables.data");
}

const std::string& ProblemInfo::getActionName(ActionIdx index) const { return actionNames.at(index); }

const std::string& ProblemInfo::getVariableName(VariableIdx index) const { return variableNames.at(index); }

unsigned ProblemInfo::getNumVariables() const { return variableNames.size(); }

const std::string ProblemInfo::getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const {
	const ObjectType generictype = variableGenericTypes.at(varIdx);
	if (generictype == ObjectType::OBJECT) return getCustomObjectName(objIdx);
	else if (generictype == ObjectType::INT) return std::to_string(objIdx);
	else if (generictype == ObjectType::BOOL) return std::string((objIdx ? "true" : "false"));
	throw std::runtime_error("Should never get here.");
}

const std::string ProblemInfo::getObjectName(const std::string& type, ObjectIdx objIdx) const {
	if (parseVariableType(type) == ObjectType::OBJECT) return getCustomObjectName(objIdx);
	else if (parseVariableType(type) == ObjectType::INT) return std::to_string(objIdx);
	else if (parseVariableType(type) == ObjectType::BOOL) return std::string((objIdx ? "true" : "false"));
	throw std::runtime_error("Should never get here.");	
}

const std::string& ProblemInfo::getCustomObjectName(ObjectIdx objIdx) const { return objectNames.at(objIdx); }

unsigned ProblemInfo::getNumObjects() const { return objectNames.size(); }


void ProblemInfo::loadVariableIndex(const std::string& filename) {
	std::string line;
	std::ifstream in(filename);
	
	// Parse the names of the state variables - each line is a variable.
	while (std::getline(in, line)) {
		// line is of the form "16.ontable(d)#_bool_"
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("#"));
		assert(strs.size()==2);
		
		unsigned point = strs[0].find(".");
		assert((unsigned)std::stoi(strs[0].substr(0, point)) == variableNames.size());
		variableNames.push_back(strs[0].substr(point + 1));
		variableGenericTypes.push_back(parseVariableType(strs[1]));
		variableTypes.push_back(name_to_type.at(strs[1]));
	}
}

ProblemInfo::ObjectType ProblemInfo::parseVariableType(const std::string& type) const {
	if (isTypeBounded[getTypeId(type)] || type == "_int_" || type == "int") return ObjectType::INT;
	else if (type == "_bool_" || type == "bool") return ObjectType::BOOL;
	else return ObjectType::OBJECT;
}

//! Load the names of the state variables from the specified file.
void ProblemInfo::loadActionIndex(const std::string& filename) {
	std::string line;
	std::ifstream in(filename);
	
	// Parse the names of the ground actions - each line is an action name.
	while (std::getline(in, line)) {
		actionNames.push_back(line); // The action line is implicitly the action index.
	}
}

//! Load the names of the problem objects from the specified file.
void ProblemInfo::loadObjectIndex(const std::string& filename) {
	std::string line;
	std::ifstream in(filename);
	
	// Each line is an object name.
	while (std::getline(in, line)) {
		objectIds.insert(std::make_pair(line, objectNames.size()));
		objectNames.push_back(line); // The action line is implicitly the action index.
	}
}

void ProblemInfo::loadTypeObjects(const std::string& filename) {
	std::string line;
	std::ifstream in(filename);
	
	typeObjects.resize(name_to_type.size()); // Resize the vector to the number of types that we have
	isTypeBounded.resize(name_to_type.size());
	typeBounds.resize(name_to_type.size());
	
	// Each line is an object name.
	while (std::getline(in, line)) {
		// line is of the form "tdirection#5,6,7,8,9,10,11,12"
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("#"));
		assert(strs.size()==2);
		if (strs[1].size() == 0) throw std::runtime_error("No objects declared in " + filename + " for type " + strs[0]);
		
		TypeIdx type_id = name_to_type.at(strs[0]);
		assert(type_id < typeObjects.size());
		
		// We read and convert to integer type the vector of Object indexes
		// strs[1] is either of the form (a) "5,6,7,8" or (b) "int[0..10]"
		boost::match_results<std::string::const_iterator> results;
		if (boost::regex_match(strs[1], results, boundedIntRE)) { // We have a bounded integer domain
			int lower = boost::lexical_cast<int>(results[1]);
			int upper = boost::lexical_cast<int>(results[2]);
			if (lower > upper) throw std::runtime_error("Incorrect bounded integer expression " + line);
			typeBounds[type_id] = std::make_pair(lower, upper);
			isTypeBounded[type_id] = true;
			
			// Unfold the range
			std::vector<ObjectIdx> values;
			values.reserve(upper - lower + 1);
			for (int v = lower; v <= upper; ++v) values.push_back(v);
			typeObjects[type_id] = values;
		} else { // We have an enumeration of object IDs
			std::vector<std::string> string_indexes;
			std::vector<ObjectIdx> indexes;
			boost::split(string_indexes, strs[1], boost::is_any_of(","));
			indexes.reserve(string_indexes.size());
			for (auto& str:string_indexes) indexes.push_back(boost::lexical_cast<ObjectIdx>(str));
			typeObjects[type_id] = indexes;
		}
	}
}

void ProblemInfo::loadTypeIndex(const std::string& filename) {
	std::string line;
	std::ifstream in(filename);
	
	// Each line is an object name.
	while (std::getline(in, line)) {
		// line is of the form "0#typename"
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("#"));
		assert(strs.size()==2);
		assert(type_to_name.size() == boost::lexical_cast<unsigned>(strs[0]));
		name_to_type.insert(std::make_pair(strs[1], type_to_name.size()));
		type_to_name.push_back(strs[1]);
	}
}

bool ProblemInfo::checkValueIsValid(const Fact& atom) const {
	return checkValueIsValid(atom.getVariable(), atom.getValue());
}

bool ProblemInfo::checkValueIsValid(VariableIdx variable, ObjectIdx value) const {
	TypeIdx type = getVariableType(variable);
	if (!isTypeBounded[type]) return true;
	const auto& bounds = typeBounds[type];
	return value >= bounds.first && value <= bounds.second;
}

} // namespaces
