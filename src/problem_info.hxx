
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/regex.hpp>
#include <fs0_types.hxx>
#include <iostream>

#include "lib/rapidjson/document.h"

namespace fs0 {

class Atom;

/**
  * A ProblemInfo instance holds all the relevant information about the problem, including the names and types of state variables, problem objects, etc.
  */
class ProblemInfo
{
public:
	typedef std::shared_ptr<ProblemInfo> ptr;
	typedef std::shared_ptr<const ProblemInfo> cptr;
	
	enum class ObjectType {INT, BOOL, OBJECT}; 

protected:
	//! A map from action index to action name
	std::vector<std::string> actionNames;
	
	//! A map from state variable index to action name
	std::vector<std::string> variableNames;
	
	//! A map from state variable index to the type of the state variable
	std::vector<ObjectType> variableGenericTypes;
	
	//! Maps variable index to type index
	std::vector<TypeIdx> variableTypes;
	
	//! A map from object index to object name
	std::vector<std::string> objectNames;
	std::map<std::string, ObjectIdx> objectIds;
	
	//! A map from type ID to all of the object indexes of that type
	std::vector<ObjectIdxVector> typeObjects;
	
	//! An integer type will have associated lower and upper bounds.
	std::vector<std::pair<int, int>> typeBounds;
	std::vector<bool> isTypeBounded;
	
	//! Maps between typenames and type IDs.
	std::unordered_map<std::string, TypeIdx> name_to_type;
	std::vector<std::string> type_to_name;
	
	//! The names of the problem domain and instance
	std::string _domain;
	std::string _instance;
	
public:

	ProblemInfo(const rapidjson::Document& data);
	~ProblemInfo() {}
	
	const std::string& getActionName(ActionIdx index) const;
	
	const std::string& getVariableName(VariableIdx index) const;
	
	const TypeIdx getVariableType(VariableIdx index) const { return variableTypes.at(index); }
	const ObjectType 	getVariableGenericType(VariableIdx index) const { return variableGenericTypes.at(index); }
	
	unsigned getNumVariables() const;
	
	const std::string getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const;
	const std::string getObjectName(const std::string& type, ObjectIdx objIdx) const;
	inline ObjectIdx getObjectId(const std::string& name) const { return objectIds.at(name); }
	
	//! Returns all the objects of the given type _or of a descendant type_
	inline const ObjectIdxVector& getTypeObjects(TypeIdx type) const { return typeObjects.at(type); }
	inline const ObjectIdxVector& getTypeObjects(const std::string& type_name) const { return typeObjects.at(getTypeId(type_name)); }

	//! Returns all the objects of the type of the given variable
	inline const ObjectIdxVector& getVariableObjects(const VariableIdx variable) const { 
		return getTypeObjects(getVariableType(variable));
	}
	
	inline TypeIdx getTypeId(const std::string& type_name) const {
		// MRJ: Pre-processor guards included for efficiency
		try {
			return name_to_type.at(type_name);
		} catch ( std::out_of_range& ex ) {
			std::cout << type_name << " not in type dictionary!" << std::endl;
			return 0;
		}
	}
	
	const std::string& getCustomObjectName(ObjectIdx objIdx) const;
	
	unsigned getNumObjects() const;
	
	//! Both methods check that the value of a given variable is within the bounds of the variable,
	//! in case it is a variable of a bounded type.
	bool checkValueIsValid(const Atom& atom) const;
	bool checkValueIsValid(VariableIdx variable, ObjectIdx value) const;
	
	bool hasVariableBoundedDomain(VariableIdx variable) const { return isTypeBounded[getVariableType(variable)];  }
	std::pair<int,int> getVariableBounds( VariableIdx variable ) const {
		assert(hasVariableBoundedDomain(variable));
		return typeBounds[getVariableType(variable)];
	}
	
	void setDomainName(const std::string& domain) { _domain = domain; }
	void setInstanceName(const std::string& instance) { _instance = instance; }
	const std::string& getDomainName() const { return _domain; }
	const std::string& getInstanceName() const { return _instance; }

protected:
	
	//! Load the names of the state variables from the specified file.
	void loadVariableIndex(const rapidjson::Value& data);
	
	ObjectType parseVariableType(const std::string& str) const;
	
	//! Load the names of the (bound) actions from the specified file.
	void loadActionIndex(const rapidjson::Value& data);
	
	//! Load the names of the problem objects from the specified file.
	void loadObjectIndex(const rapidjson::Value& data);
	
	//! Load all type-related info.
	void loadTypeIndex(const rapidjson::Value& data);
	
	void loadProblemMetadata(const rapidjson::Value& data);
};

} // namespaces
