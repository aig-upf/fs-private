
#pragma once

#include <string>
#include <vector>
#include <iosfwd>
#include <fstream>
#include <cassert>

#include <actions.hxx>
#include <core_types.hxx>
#include <boost/algorithm/string.hpp>



namespace aptk { namespace core {
	
class ProblemInfo
{
public:
	typedef std::shared_ptr<ProblemInfo> ptr;
	typedef std::shared_ptr<const ProblemInfo> cptr;
	
	enum class ObjectType {INT, BOOL, OBJECT};  

	ProblemInfo(const std::string& data_dir) {
		loadActionIndex(data_dir + "/action-index.data");
		loadVariablesIndex(data_dir + "/variables.data");
		loadObjectIndex(data_dir + "/objects.data");
	}
	~ProblemInfo() {}
	
	const std::string& getActionName(ActionIdx index) const {
		return actionNames.at(index);
	}
	
	const std::string& getVariableName(VariableIdx index) const {
		return variableNames.at(index);
	}
	
	const ObjectType getVariableType(VariableIdx index) const {
		return variableTypes.at(index);
	}	
	
	unsigned getNumVariables() const {
		return variableNames.size();
	}
	
	const std::string getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const {
		if (getVariableType(varIdx) == ObjectType::OBJECT) return getCustomObjectName(objIdx);
		else if (getVariableType(varIdx) == ObjectType::INT) return std::to_string(objIdx);
		else if (getVariableType(varIdx) == ObjectType::BOOL) return std::string((objIdx ? "true" : "false"));
		throw std::runtime_error("Should never get here.");
	}
	
	const std::string& getCustomObjectName(ObjectIdx objIdx) const {
		return objectNames.at(objIdx);
	}
	
	unsigned getNumObjects() const {
		return objectNames.size();
	}

protected:
	
	//! A map from action index to action name
	std::vector<std::string> actionNames;
	
	//! A map from state variable index to action name
	std::vector<std::string> variableNames;
	
	//! A map from state variable index to action name
	std::vector<ProblemInfo::ObjectType> variableTypes;
	
	//! A map from object index to object name
	std::vector<std::string> objectNames;
	
	
	//! Load the names of the (bound) actions from the specified file.
	void loadVariablesIndex(const std::string& filename) {
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
		}
	}
	
	ObjectType parseVariableType(const std::string& str) {
		if (str == "_int_" || str == "int") return ObjectType::INT;
		else if (str == "_bool_" || str == "bool") return ObjectType::BOOL;
		else return ObjectType::OBJECT;
	}
	
	//! Load the names of the state variables from the specified file.
	void loadActionIndex(const std::string& filename) {
		std::string line;
		std::ifstream in(filename);
		
		// Parse the names of the ground actions - each line is an action name.
		while (std::getline(in, line)) {
			actionNames.push_back(line); // The action line is implicitly the action index.
		}
	}
	
	//! Load the names of the problem objects from the specified file.
	void loadObjectIndex(const std::string& filename) {
		std::string line;
		std::ifstream in(filename);
		
		// Each line is an object name.
		while (std::getline(in, line)) {
			objectNames.push_back(line); // The action line is implicitly the action index.
		}
	}	
	
};

	  
	  
	  
} } // namespaces
