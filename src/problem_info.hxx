
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/regex.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <fs0_types.hxx>
#include <iostream>

#include <lib/rapidjson/document.h>

namespace fs0 {

class Atom;

//! Data related to the functional symbols
class FunctionData {
public:
	
	FunctionData(const Signature& signature, TypeIdx codomain, std::vector<VariableIdx>& variables, bool stat):
		_signature(signature), _codomain(codomain), _variables(variables), _static(stat) {}
	
	//! Returns the state variables derived from the given function (e.g. for a function "f", f(1), f(2), ...)
	const std::vector<VariableIdx>& getStateVariables() const {
		assert(!_static);
		return _variables;
	}
	
	const Signature& getSignature() const { return _signature; }
	const TypeIdx& getCodomainType() const { return _codomain; }
	
	bool isStatic() const { return _static; }
	
	//! Sets/Gets the actual implementation of the function
	void setFunction(const Function& function) {
		assert(_static);
		_function = function;
	}
	const Function& getFunction() const { 
		assert(_function);
		return _function;
	}

protected:
	Signature _signature;
	TypeIdx _codomain;
	std::vector<VariableIdx> _variables;
	bool _static;
	
	//! The actual implementation of the function
	Function _function;
};

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
	//! A map from state variable ID to state variable name
	std::vector<std::string> variableNames;
	
	//! A map from state variable name to state variable ID
	std::map<std::string, VariableIdx> variableIds;
	
	//! A map from the actual data "f(t1, t2, ..., tn)" to the assigned variable ID
	std::map<std::pair<unsigned, std::vector<ObjectIdx>>, VariableIdx> variableDataToId;
	
	//! A map from state variable index to the type of the state variable
	std::vector<ObjectType> variableGenericTypes;
	
	//! Maps variable index to type index
	std::vector<TypeIdx> variableTypes;
	
	//! A map from object index to object name
	std::vector<std::string> objectNames;
	
	//! A map from object name to object index
	std::map<std::string, ObjectIdx> objectIds;
	
	//! A map from type ID to all of the object indexes of that type
	std::vector<ObjectIdxVector> typeObjects;
	
	//! An integer type will have associated lower and upper bounds.
	std::vector<std::pair<int, int>> typeBounds;
	std::vector<bool> isTypeBounded;
	
	//! Maps between typenames and type IDs.
	std::unordered_map<std::string, TypeIdx> name_to_type;
	std::vector<std::string> type_to_name;
	
	//! A map from function ID to function name
	std::vector<std::string> functionNames;
	
	//! A map from function name to function ID
	std::map<std::string, unsigned> functionIds;
	
	//! A map from function ID to the function data
	std::vector<FunctionData> functionData;
	
	//! The names of the problem domain and instance
	std::string _domain;
	std::string _instance;
	
public:

	ProblemInfo(const rapidjson::Document& data);
	~ProblemInfo() {}
	
	const std::string& getVariableName(VariableIdx index) const;
	inline VariableIdx getVariableId(const std::string& name) const { return variableIds.at(name); }
	VariableIdx getVariableId(unsigned symbol_id, const std::vector<ObjectIdx>& subterms) const;
	
	const TypeIdx getVariableType(VariableIdx index) const { return variableTypes.at(index); }
	const ObjectType getVariableGenericType(VariableIdx index) const { return variableGenericTypes.at(index); }
	
	unsigned getNumVariables() const;
	
	const std::string getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const;
	const std::string deduceObjectName(ObjectIdx object, TypeIdx type) const;
	inline ObjectIdx getObjectId(const std::string& name) const { return objectIds.at(name); }
	
	//! Return the ID of the function with given name
	inline unsigned getFunctionId(const std::string& name) const { return functionIds.at(name); }
	const std::string& getFunctionName(unsigned function_id) const { return functionNames.at(function_id); }
	void setFunction(unsigned functionId, const Function& function) {
		functionData.at(functionId).setFunction(function);
	}
	inline const FunctionData& getFunctionData(unsigned functionId) const { return functionData.at(functionId); }
	
	//! Returns all the objects of the given type _or of a descendant type_
	inline const std::vector<ObjectIdxVector>& getTypeObjects() const { return typeObjects; }
	inline const ObjectIdxVector& getTypeObjects(TypeIdx type) const { return typeObjects.at(type); }
	inline const ObjectIdxVector& getTypeObjects(const std::string& type_name) const { return typeObjects.at(getTypeId(type_name)); }

	//! Returns all the objects of the type of the given variable
	inline const ObjectIdxVector& getVariableObjects(const VariableIdx variable) const { 
		return getTypeObjects(getVariableType(variable));
	}
	
	inline TypeIdx getTypeId(const std::string& type_name) const {
		auto it = name_to_type.find(type_name);
		if (it == name_to_type.end()) {
			throw std::runtime_error("Unknown object type " + type_name);
		}
		return it->second;
	}
	
	inline const std::string& getTypename(TypeIdx type) const { return type_to_name.at(type); }
	
	//! Resolves a pair of function ID + an assignment of values to their parameters to the corresponding state variable.
	VariableIdx resolveStateVariable(unsigned symbol_id, std::vector<ObjectIdx>&& constants) const { return variableDataToId.at(std::make_pair(symbol_id, constants)); }
	VariableIdx resolveStateVariable(unsigned symbol_id, const std::vector<ObjectIdx>& constants) const { return variableDataToId.at(std::make_pair(symbol_id, constants)); }
	
	//! Resolves a function ID to all state variables in which the function can result
	const VariableIdxVector& resolveStateVariable(unsigned symbol_id) const { return getFunctionData(symbol_id).getStateVariables(); }
	
	
	const std::string& getCustomObjectName(ObjectIdx objIdx) const;
	
	unsigned getNumObjects() const;
	
	//! Both methods check that the value of a given variable is within the bounds of the variable,
	//! in case it is a variable of a bounded type.
	bool checkValueIsValid(const Atom& atom) const;
	bool checkValueIsValid(VariableIdx variable, ObjectIdx value) const;
	
	bool isBoundedType(TypeIdx type) const { return isTypeBounded[type];  }
	bool isBoundedVariable(VariableIdx variable) const { return isBoundedType(getVariableType(variable));  }
	
	const std::pair<int,int>& getTypeBounds(TypeIdx type) const {
		assert(isBoundedType(type));
		return typeBounds.at(type);
	}
	
	const std::pair<int,int>& getVariableBounds(VariableIdx variable) const { return getTypeBounds(getVariableType(variable)); }
	
	void setDomainName(const std::string& domain) { _domain = domain; }
	void setInstanceName(const std::string& instance) { _instance = instance; }
	const std::string& getDomainName() const { return _domain; }
	const std::string& getInstanceName() const { return _instance; }
	
	//! Returns the generic type (object, int, bool, etc.) corresponding to a concrete type
	ObjectType getGenericType(TypeIdx typeId) const;
	
	//! Returns a vector 'v' such that 'v[i]' contains all posible values for the type 'signature[i]'
	std::vector<const ObjectIdxVector*> getSignatureValues(const Signature& signature) const;
	

protected:
	
	//! Load all the function-related data 
	void loadFunctionIndex(const rapidjson::Value& data);
	
	//! Load the names of the state variables
	void loadVariableIndex(const rapidjson::Value& data);
	
	//! Load the names of the problem objects
	void loadObjectIndex(const rapidjson::Value& data);
	
	//! Load all type-related info.
	void loadTypeIndex(const rapidjson::Value& data);
	
	void loadProblemMetadata(const rapidjson::Value& data);
};

} // namespaces
