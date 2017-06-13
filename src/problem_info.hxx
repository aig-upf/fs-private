
#pragma once

#include <fs_types.hxx>
#include <unordered_map>

#include <lib/rapidjson/document.h>
#include <utils/static.hxx>
#include <utils/external.hxx>

namespace fs0 { namespace fstrips { class LanguageInfo; }}

namespace fs0 {

class Atom;

//! Data related to function and predicate symbols
class SymbolData {
public:

	enum class Type {PREDICATE, FUNCTION};

	SymbolData(Type type, const Signature& signature, TypeIdx codomain, bool stat, bool unbounded):
		_type(type), _signature(signature), _codomain(codomain), _static(stat), _unbounded_arity(unbounded) {}

	Type getType() const { return _type; }
	const Signature& getSignature() const { return _signature; }
	const TypeIdx& getCodomainType() const { return _codomain; }
	unsigned getArity() const { return _signature.size(); }

	bool isStatic() const { return _static; }

    bool hasUnboundedArity() const { return _unbounded_arity; }

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
	bool _static;
    bool _unbounded_arity;

	//! The actual implementation of the function
	Function _function;
};

/**
  * A ProblemInfo instance holds all the relevant information about the problem, including the names and types of state variables, problem objects, etc.
  */
class ProblemInfo {
public:
	enum class ObjectType {INT, BOOL, OBJECT};

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
	
	const fstrips::LanguageInfo& get_language_info() const { throw std::runtime_error("UNIMPLEMENTED"); }

protected:
	//! The singleton instance
	static std::unique_ptr<ProblemInfo> _instance;

	//! A map from state variable ID to state variable name
	std::vector<std::string> variableNames;

	//! A map from state variable name to state variable ID
	std::unordered_map<std::string, VariableIdx> variableIds;

	//! A map from the actual data "f(t1, t2, ..., tn)" to the assigned variable ID
	std::map<std::pair<unsigned, std::vector<object_id>>, VariableIdx> variableDataToId;
	std::vector<std::pair<unsigned, std::vector<object_id>>> variableIdToData;

	//! Mapping from state variable index to the type associated to the state variable
	std::vector<type_id> _sv_types;

	//! Maps variable index to type index
	std::vector<TypeIdx> variableTypes;

	//! A map from object index to object name
	std::unordered_map<object_id, std::string> objectNames;

	//! A map from object name to object index
	std::unordered_map<std::string, object_id> objectIds;

	//! A map from type ID to all of the object indexes of that type
	std::vector<std::vector<object_id>> typeObjects;

	//! An integer type will have associated lower and upper bounds.
	std::vector<std::pair<int, int>> typeBounds;
	std::vector<bool> isTypeBounded;

	//! Maps between typenames and type IDs.
	std::unordered_map<std::string, TypeIdx> name_to_type;
	std::vector<std::string> type_to_name;

	//! Maps between predicate and function symbols names and IDs.
	std::vector<std::string> symbolNames;
	std::unordered_map<std::string, SymbolIdx> symbolIds;

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
	
	type_id sv_type(VariableIdx var) const { return _sv_types.at(var); }

	unsigned getNumVariables() const;

	
	std::string object_name(const object_id& object) const;
	const std::string& custom_object_name(const object_id& objIdx) const;
	
	inline object_id getObjectId(const std::string& name) const { return objectIds.at(name); }

	//! Return the ID of the function with given name
	inline SymbolIdx getSymbolId(const std::string& name) const { return symbolIds.at(name); }
	const std::string& getSymbolName(unsigned symbol_id) const { return symbolNames.at(symbol_id); }
	const std::vector<std::string>& getSymbolNames() const { return symbolNames; }
	unsigned getNumLogicalSymbols() const { return symbolIds.size(); }
	bool isPredicate(unsigned symbol_id) const { return getSymbolData(symbol_id).getType() == SymbolData::Type::PREDICATE; }
	bool isFunction(unsigned symbol_id) const { return getSymbolData(symbol_id).getType() == SymbolData::Type::FUNCTION; }

	bool isPredicativeVariable(VariableIdx variable) const { return _predicative_variables.at(variable); }

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
	inline const std::vector<std::vector<object_id>>& getTypeObjects() const { return typeObjects; }
	inline const std::vector<object_id>& getTypeObjects(TypeIdx type) const { return typeObjects.at(type); }
	inline const std::vector<object_id>& getTypeObjects(const std::string& type_name) const { return typeObjects.at(getTypeId(type_name)); }

	//! Returns all the objects of the type of the given variable
	inline const std::vector<object_id>& getVariableObjects(const VariableIdx variable) const {
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
	VariableIdx resolveStateVariable(unsigned symbol_id, std::vector<object_id>&& constants) const { return variableDataToId.at(std::make_pair(symbol_id, std::move(constants))); }
	VariableIdx resolveStateVariable(unsigned symbol_id, const std::vector<object_id>& constants) const { return variableDataToId.at(std::make_pair(symbol_id, constants)); }

	//! Return the data that originated a state variable
	const std::pair<unsigned, std::vector<object_id>>& getVariableData(VariableIdx variable) const { return variableIdToData.at(variable); }

	unsigned num_objects() const;

	//! Both methods check that the value of a given variable is within the bounds of the variable,
	//! in case it is a variable of a bounded type.
	bool checkValueIsValid(const Atom& atom) const;
	bool checkValueIsValid(VariableIdx variable, const object_id& value) const;

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
	// TODO REMOVE IN FAVOR OF get_type_id
	ObjectType getGenericType(TypeIdx typeId) const;
	ObjectType getGenericType(const std::string& type) const;
	
	//! Return the generic type_id corresponding to the given fs-type
	type_id get_type_id(const std::string& fstype) const;
	type_id get_type_id(TypeIdx fstype) const;


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
