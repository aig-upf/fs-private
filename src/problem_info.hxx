
#pragma once

#include <fs_types.hxx>
#include <unordered_map>

#include <lib/rapidjson/document.h>
#include <utils/static.hxx>
#include <utils/external.hxx>

namespace fs0 {

class Atom;

//! Data related to function and predicate symbols
class SymbolData {
public:

	enum class Type {PREDICATE, FUNCTION};

	SymbolData(Type type, const Signature& signature, TypeIdx codomain, std::vector<VariableIdx>& variables, bool stat):
		_type(type), _signature(signature), _codomain(codomain), _variables(variables), _static(stat) {}

	//! Returns the state variables derived from the given function (e.g. for a function "f", f(1), f(2), ...)
	const std::vector<VariableIdx>& getStateVariables() const {
		assert(!_static);
		return _variables;
	}

	Type getType() const { return _type; }
	const Signature& getSignature() const { return _signature; }
	const TypeIdx& getCodomainType() const { return _codomain; }
	unsigned getArity() const { return _signature.size(); }

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
	Type _type;
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
class ProblemInfo {
public:
	enum class ObjectType {INT, BOOL, OBJECT, FLOAT};

	//! Set the global singleton problem instance
	static ProblemInfo& setInstance(std::unique_ptr<ProblemInfo>&& problem) {
		assert(!_instance);
		_instance = std::move(problem);
		return *_instance;
	}

	//! Global singleton object accessor
	static const ProblemInfo& getInstance() {
		assert(_instance);
		return *_instance;
	}

protected:
	//! The singleton instance
	static std::unique_ptr<ProblemInfo> _instance;

	//! A map from state variable ID to state variable name
	std::vector<std::string> variableNames;

	//! A map from state variable name to state variable ID
	std::map<std::string, VariableIdx> variableIds;

	//! A map from the actual data "f(t1, t2, ..., tn)" to the assigned variable ID
	std::map<std::pair<unsigned, std::vector<ObjectIdx>>, VariableIdx> variableDataToId;
	std::vector<std::pair<unsigned, std::vector<ObjectIdx>>> variableIdToData;

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

	//! Maps between predicate and function symbols names and IDs.
	std::vector<std::string> symbolNames;
	std::map<std::string, SymbolIdx> symbolIds;

	//! A map from function ID to the function data
	std::vector<SymbolData> _functionData;

	//! The names of the problem domain and instance
	std::string _domain;
	std::string _instance_name;

	//! The extensions of the static symbols
	std::vector<std::unique_ptr<StaticExtension>> _extensions;

	std::unique_ptr<ExternalI> _external;

public:
	ProblemInfo(const rapidjson::Document& data, const std::string& data_dir);
	~ProblemInfo() = default;

	const std::string& getVariableName(VariableIdx index) const;
	inline VariableIdx getVariableId(const std::string& name) const { return variableIds.at(name); }


	const TypeIdx getVariableType(VariableIdx index) const { return variableTypes.at(index); }
	const ObjectType getVariableGenericType(VariableIdx index) const { return variableGenericTypes.at(index); }

	unsigned getNumVariables() const;

	const std::string getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const;
	const std::string deduceObjectName(ObjectIdx object, TypeIdx type) const;
	const std::string deduceObjectName(ObjectIdx object, const std::string& type) const { return deduceObjectName(object, getTypeId(type)); }
	inline ObjectIdx getObjectId(const std::string& name) const { return objectIds.at(name); }

	//! Return the ID of the function with given name
	inline SymbolIdx getSymbolId(const std::string& name) const { return symbolIds.at(name); }
	const std::string& getSymbolName(unsigned symbol_id) const { return symbolNames.at(symbol_id); }
	const std::vector<std::string>& getSymbolNames() const { return symbolNames; }
	unsigned getNumLogicalSymbols() const { return symbolIds.size(); }
	bool isPredicate(unsigned symbol_id) const { return getSymbolData(symbol_id).getType() == SymbolData::Type::PREDICATE; }
	bool isFunction(unsigned symbol_id) const { return getSymbolData(symbol_id).getType() == SymbolData::Type::FUNCTION; }

	bool isPredicativeVariable(VariableIdx variable) const { return _predicative_variables.at(variable); }
	bool isNegatedPredicativeAtom(const Atom& atom) const;

    bool isIntegerNumber(VariableIdx x) const {
        return getVariableGenericType(x) == ProblemInfo::ObjectType::INT
                || getVariableGenericType(x) == ProblemInfo::ObjectType::OBJECT;
    }

    bool isRationalNumber(VariableIdx x) const {
        return getVariableGenericType(x) == ProblemInfo::ObjectType::FLOAT;
    }

	void setFunction(unsigned functionId, const Function& function) {
		_functionData.at(functionId).setFunction(function);
	}
	inline const SymbolData& getSymbolData(unsigned functionId) const { return _functionData.at(functionId); }


	void set_extension(unsigned symbol_id, std::unique_ptr<StaticExtension>&& extension);
	const StaticExtension& get_extension(unsigned symbol_id) const;

	void set_external(std::unique_ptr<ExternalI> external) { _external = std::move(external); }
	const ExternalI& get_external() const {
		assert(_external);
		return *_external;
	}

	//! A convenient helper
	template <typename ExtensionT>
	const ExtensionT& get_extension(const std::string& symbol) const {
		unsigned id = getSymbolId(symbol);
		assert(_extensions.at(id) != nullptr);
		const ExtensionT* extension = dynamic_cast<const ExtensionT*>(_extensions.at(id).get());
		assert(extension);
		return *extension;
	}

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
	VariableIdx resolveStateVariable(unsigned symbol_id, std::vector<ObjectIdx>&& constants) const { return variableDataToId.at(std::make_pair(symbol_id, std::move(constants))); }
	VariableIdx resolveStateVariable(unsigned symbol_id, const std::vector<ObjectIdx>& constants) const { return variableDataToId.at(std::make_pair(symbol_id, constants)); }

	//! Return the data that originated a state variable
	const std::pair<unsigned, std::vector<ObjectIdx>>& getVariableData(VariableIdx variable) const { return variableIdToData.at(variable); }



	//! Resolves a function ID to all state variables in which the function can result
	const VariableIdxVector& resolveStateVariable(unsigned symbol_id) const { return getSymbolData(symbol_id).getStateVariables(); }


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

	void setDomainName(const std::string& domain) { _domain = domain; }
	void setInstanceName(const std::string& instance) { _instance_name = instance; }
	const std::string& getDomainName() const { return _domain; }
	const std::string& getInstanceName() const { return _instance_name; }

	//! Returns the generic type (object, int, bool, etc.) corresponding to a concrete type
	ObjectType getGenericType(TypeIdx typeId) const;
	ObjectType getGenericType(const std::string& type) const;

	const std::string& getDataDir() const { return _data_dir; }

protected:
	//! Load all the function-related data
	void loadSymbolIndex(const rapidjson::Value& data);

	//! Load the names of the state variables
	void loadVariableIndex(const rapidjson::Value& data);

	//! Load the names of the problem objects
	void loadObjectIndex(const rapidjson::Value& data);

	//! Load all type-related info.
	void loadTypeIndex(const rapidjson::Value& data);

	void loadProblemMetadata(const rapidjson::Value& data);

	std::vector<bool> _predicative_variables;

	//! The filesystem directory where the problem serialized data is found
	const std::string _data_dir;
};

} // namespaces
