
#include <iostream>
#include <fstream>

#include <problem_info.hxx>
#include <boost/algorithm/string.hpp>
#include <utils/lexical_cast.hxx>
#include <atom.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 {

class unregistered_object_error : public std::runtime_error {
public:
	unregistered_object_error(const object_id& o) : std::runtime_error(msg(o)) {}
	std::string msg(const object_id& o) {
		return "Unregistered object"; // TODO ENHANCE THIS
	}
};


std::unique_ptr<ProblemInfo> ProblemInfo::_instance = nullptr;

ProblemInfo::ProblemInfo(const rapidjson::Document& data, const std::string& data_dir) :
	_data_dir(data_dir)
{

	LPT_INFO("main", "Loading Type index...");
	loadTypeIndex(data["types"]); // Order matters

	LPT_INFO("main", "Loading Object index...");
	loadObjectIndex(data["objects"]);

	LPT_INFO("main", "Loading Symbol index...");
	loadSymbolIndex(data["symbols"]);

	LPT_INFO("main", "Loading Variable index...");
	loadVariableIndex(data["variables"]);

	LPT_INFO("main", "Loading Metadata...");
	loadProblemMetadata(data["problem"]);

	LPT_INFO("main", "All indexes loaded");

	// Load the cached map of predicative variables for more performant access
	for (unsigned variable = 0; variable < getNumVariables(); ++variable) {
		_predicative_variables.push_back(isPredicate(getVariableData(variable).first));
	}

	_extensions.resize(getNumLogicalSymbols());
}

const std::string& ProblemInfo::getVariableName(VariableIdx index) const { return variableNames.at(index); }


unsigned ProblemInfo::getNumVariables() const { return variableNames.size(); }



std::string ProblemInfo::object_name(const object_id& object) const {
	type_id t = o_type(object);
	if (t == type_id::bool_t) return std::string((fs0::value<bool>(object) ? "true" : "false"));
	else if (t == type_id::int_t) return std::to_string(fs0::value<int>(object));
	else if (t == type_id::object_t) return custom_object_name(object);
	throw std::runtime_error("Unaccounted-for type");
}

std::string ProblemInfo::object_name(const object_id& object, ObjectType type) const {
	if (type == ObjectType::OBJECT) return custom_object_name(object);
	else if (type == ObjectType::INT) return std::to_string(fs0::value<int>(object, ObjectTable::EMPTY_TABLE));
	else if (type == ObjectType::BOOL) return std::string((fs0::value<bool>(object, ObjectTable::EMPTY_TABLE) ? "true" : "false"));
	throw std::runtime_error("Should never get here.");
}


const std::string& ProblemInfo::custom_object_name(object_id o) const {
	const auto& it = objectNames.find(o);
	if (it == objectNames.end()) throw unregistered_object_error(o);
	return it->second;
}

unsigned ProblemInfo::num_objects() const { return objectNames.size(); }


void ProblemInfo::loadVariableIndex(const rapidjson::Value& data) {
	assert(variableNames.empty());

	for (unsigned i = 0; i < data.Size(); ++i) {
		unsigned id = variableNames.size();
		assert(data[i]["id"].GetInt() >= 0 && static_cast<unsigned>(data[i]["id"].GetInt()) == id); // Check values are decoded in the proper order

		const std::string type(data[i]["type"].GetString());
		const std::string name(data[i]["name"].GetString());
		variableNames.push_back(name);
		variableIds.insert(std::make_pair(name, id));

		variableGenericTypes.push_back(getGenericType(type));
		_sv_types.push_back(get_type_id(type));
		try {
			variableTypes.push_back(name_to_type.at(type));
		} catch( std::out_of_range& ex ) {
			throw std::runtime_error("Unknown type " + type);
		}

		// Load the info necessary to resolve state variables dynamically
		const auto& var_data = data[i]["data"];
		unsigned symbol_id = var_data[0].GetInt();
		 std::vector<object_id> constants;
		for (unsigned j = 0; j < var_data[1].Size(); ++j) {
			constants.push_back(make_obj(var_data[1][j].GetInt()));
		}

		variableDataToId.insert(std::make_pair(std::make_pair(symbol_id, constants),  id));
		variableIdToData.push_back(std::make_pair(symbol_id, constants));
	}
}

void ProblemInfo::loadSymbolIndex(const rapidjson::Value& data) {
	assert(symbolIds.empty());


	// Symbol data is stored as: # <symbol_id, symbol_name, symbol_type, <function_domain>, function_codomain, state_variables, static?>
	for (unsigned i = 0; i < data.Size(); ++i) {
		const unsigned id = data[i][0].GetInt();
		const std::string name(data[i][1].GetString());
		assert(id == symbolIds.size()); // Check values are decoded in the proper order
		symbolIds.insert(std::make_pair(name, id));
		symbolNames.push_back(name);

		// Parse the symbol type: function or predicate
		const std::string symbol_type = data[i][2].GetString();
		assert (symbol_type == "function" || symbol_type == "predicate");
		const SymbolData::Type type = (symbol_type == "function") ? SymbolData::Type::FUNCTION : SymbolData::Type::PREDICATE;

		// Parse the domain IDs
		const auto& domains = data[i][3];
		Signature domain;
		for (unsigned j = 0; j < domains.Size(); ++j) {
			domain.push_back(getTypeId(domains[j].GetString()));
		}

		// Parse the codomain ID
		TypeIdx codomain = getTypeId(data[i][4].GetString());

		// Parse the function variables
		std::vector<VariableIdx> variables;
		const auto& list = data[i][5];
		for (unsigned j = 0; j < list.Size(); ++j) {
			variables.push_back(list[j][0].GetInt());
		}

		bool is_static = data[i][6].GetBool();
        bool has_unbounded_arity = data[i][7].GetBool();
		_functionData.push_back(SymbolData(type, domain, codomain, variables, is_static, has_unbounded_arity));
	}
}


ProblemInfo::ObjectType ProblemInfo::getGenericType(TypeIdx typeId) const {
	if (isTypeBounded[typeId]) return ObjectType::INT;
	else return ObjectType::OBJECT;
}

ProblemInfo::ObjectType ProblemInfo::getGenericType(const std::string& type) const {
	if (type == "bool") return ObjectType::BOOL;
	return getGenericType(getTypeId(type));
}

type_id ProblemInfo::
get_type_id(const std::string& type) const {
	if (type == "bool") return type_id::bool_t;
	if (isTypeBounded[getTypeId(type)]) return type_id::int_t;
	else return type_id::object_t;
}

//! Load the names of the problem objects from the specified file.
void ProblemInfo::loadObjectIndex(const rapidjson::Value& data) {
	assert(objectNames.empty());

	for (unsigned i = 0; i < data.Size(); ++i) {
		assert(data[i]["id"].GetInt() >= 0 && static_cast<unsigned>(data[i]["id"].GetInt()) == objectNames.size()); // Check values are decoded in the proper order
		const std::string& name = data[i]["name"].GetString();
		object_id oid = make_obj<int>(objectNames.size());
		objectIds.insert(std::make_pair(name, oid));
		objectNames.insert(std::make_pair(oid, name));
// 		std::cout << "Inserted: " << oid << " - " << name << " [" << std::hash<object_id>()(oid) << "]" << std::endl;
	}
}


void ProblemInfo::loadTypeIndex(const rapidjson::Value& data) {
	assert(type_to_name.empty());
	unsigned num_types = data.Size();

	typeObjects.resize(num_types); // Resize the vector to the number of types that we have
	isTypeBounded.resize(num_types);
	typeBounds.resize(num_types);

	for (unsigned i = 0; i < data.Size(); ++i) {
		TypeIdx type_id = data[i][0].GetInt();
		std::string type_name(data[i][1].GetString());
		assert(type_id == type_to_name.size()); // Check values are decoded in the proper order

		name_to_type.insert(std::make_pair(type_name, type_id));
		type_to_name.push_back(type_name);

		// We read and convert to integer type the vector of Object indexes
		if (data[i][2].IsString()) {
			assert(std::string(data[i][2].GetString()) == "int" && data[i].Size() == 4);
			int lower = data[i][3][0].GetInt();
			int upper = data[i][3][1].GetInt();
			if (lower > upper) throw std::runtime_error("Incorrect bounded integer expression: [" + std::to_string(lower) + ", " + std::to_string(upper) + "]");

			typeBounds[type_id] = std::make_pair(lower, upper);
			isTypeBounded[type_id] = true;

			// Unfold the range
			typeObjects[type_id].reserve(upper - lower + 1);
			for (int v = lower; v <= upper; ++v) typeObjects[type_id].push_back(make_obj(v));
		} else { // We have an enumeration of object IDs
			typeObjects[type_id].reserve(data[i][2].Size());
			for (unsigned j = 0; j < data[i][2].Size(); ++j) {
				int value = boost::lexical_cast<int>(data[i][2][j].GetString());
				typeObjects[type_id].push_back(make_obj(value));
			}
		}
	}
}

bool ProblemInfo::checkValueIsValid(const Atom& atom) const {
	return checkValueIsValid(atom.getVariable(), atom.getValue());
}

bool ProblemInfo::checkValueIsValid(VariableIdx variable, object_id object) const {
	TypeIdx type = getVariableType(variable);
	if (!isTypeBounded[type]) return true;
	const auto& bounds = typeBounds[type];
	int value = fs0::value<int>(object, ObjectTable::EMPTY_TABLE);
	return value >= bounds.first && value <= bounds.second;
}


void ProblemInfo::loadProblemMetadata(const rapidjson::Value& data) {
	setDomainName(data["domain"].GetString());
	setInstanceName(data["instance"].GetString());
}


void
ProblemInfo::set_extension(unsigned symbol_id, std::unique_ptr<StaticExtension>&& extension) {
	assert(_extensions.at(symbol_id) == nullptr); // Shouldn't be setting twice the same extension
	setFunction(symbol_id, extension->get_function());
	_extensions.at(symbol_id) = std::move(extension);
}


const StaticExtension&
ProblemInfo::get_extension(unsigned symbol_id) const {
	assert(_extensions.at(symbol_id) != nullptr);
	return *_extensions.at(symbol_id);
}


} // namespaces
