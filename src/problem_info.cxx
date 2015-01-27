
#include <problem_info.hxx>

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace fs0 {

ProblemInfo::ProblemInfo(const std::string& data_dir) {
	loadActionIndex(data_dir + "/action-index.data");
	loadVariableIndex(data_dir + "/variables.data");
	loadObjectIndex(data_dir + "/objects.data");
	loadTypeObjects(data_dir + "/object-types.data");
}

const std::string& ProblemInfo::getActionName(ActionIdx index) const { return actionNames.at(index); }

const std::string& ProblemInfo::getVariableName(VariableIdx index) const { return variableNames.at(index); }

const ProblemInfo::ObjectType ProblemInfo::getVariableType(VariableIdx index) const { return variableTypes.at(index); }

unsigned ProblemInfo::getNumVariables() const { return variableNames.size(); }

const std::string ProblemInfo::getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const {
	if (getVariableType(varIdx) == ObjectType::OBJECT) return getCustomObjectName(objIdx);
	else if (getVariableType(varIdx) == ObjectType::INT) return std::to_string(objIdx);
	else if (getVariableType(varIdx) == ObjectType::BOOL) return std::string((objIdx ? "true" : "false"));
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
		variableTypes.push_back(parseVariableType(strs[1]));
		variableTypenames.push_back(strs[1]);
	}
}

ProblemInfo::ObjectType ProblemInfo::parseVariableType(const std::string& str) {
	if (str == "_int_" || str == "int") return ObjectType::INT;
	else if (str == "_bool_" || str == "bool") return ObjectType::BOOL;
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
		objectNames.push_back(line); // The action line is implicitly the action index.
	}
}

void ProblemInfo::loadTypeObjects(const std::string& filename) {
	std::string line;
	std::ifstream in(filename);
	
	// Each line is an object name.
	while (std::getline(in, line)) {
		// line is of the form "tdirection#5,6,7,8,9,10,11,12"
		std::vector<std::string> strs;
		boost::split(strs, line, boost::is_any_of("#"));
		assert(strs.size()==2);
		
		// We read and convert to integer type the vector of Object indexes
		std::vector<std::string> string_indexes;
		std::vector<ObjectIdx> indexes;
		boost::split(string_indexes, strs[1], boost::is_any_of(","));
		indexes.resize(string_indexes.size());
		for (auto& str:string_indexes) indexes.push_back(boost::lexical_cast<ObjectIdx>(str));
		typeObjects[strs[0]] = indexes;
	}
}


} // namespaces
