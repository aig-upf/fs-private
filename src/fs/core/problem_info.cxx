
#include <iostream>
#include <fstream>

#include <fs/core/problem_info.hxx>
#include <boost/algorithm/string.hpp>
#include <fs/core/utils/lexical_cast.hxx>
#include <fs/core/atom.hxx>
#include <lapkt/tools/logging.hxx>

#include <fs/core/fstrips/language_info.hxx>

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
	_data_dir(data_dir), _can_extensionalize_var_domains(true)
{
	LPT_INFO("main", "Loading Symbol index...");
	loadSymbolIndex(data["symbols"]);

	LPT_INFO("main", "Loading Variable index...");
	loadVariableIndex(data["variables"]);

	LPT_INFO("main", "Loading Metadata...");
	loadProblemMetadata(data["problem"]);

	LPT_INFO("main", "All indexes loaded");

	// Load the cached map of predicative variables for more performant access
	for (unsigned variable = 0; variable < getNumVariables(); ++variable) {
		_predicative_variables.push_back((unsigned) isPredicate(getVariableData(variable).first));
	}

	_extensions.resize(getNumLogicalSymbols());
}

const std::string& ProblemInfo::getVariableName(VariableIdx index) const { return variableNames.at(index); }


unsigned ProblemInfo::getNumVariables() const { return variableNames.size(); }



void ProblemInfo::loadVariableIndex(const rapidjson::Value& data) {
	assert(variableNames.empty());
	const fstrips::LanguageInfo lang = fstrips::LanguageInfo::instance();


	for (unsigned i = 0; i < data.Size(); ++i) {
		const auto& var_data = data[i];
		unsigned id = variableNames.size();
		assert(var_data["id"].GetInt() >= 0 && static_cast<unsigned>(var_data["id"].GetInt()) == id); // Check values are decoded in the proper order

		const std::string type(var_data["fstype"].GetString());
		const std::string name(var_data["name"].GetString());
		variableNames.push_back(name);
		variableIds.insert(std::make_pair(name, id));

		type_id t = get_type_id(type);
		_sv_types.push_back(t);

		try {
			variableTypes.push_back(lang.get_fstype_id(type));
		} catch( std::out_of_range& ex ) {
			throw std::runtime_error("Unknown FS-type " + type);
		}

		if (t == type_id::int_t) {
			_can_extensionalize_var_domains = false;
		}


		// Load the info necessary to resolve state variables dynamically
		unsigned symbol_id = var_data["symbol_id"].GetInt();

        const Signature& signature = getSymbolData(symbol_id).getSignature();
        // TODO The call above should soon be replaced by:
//		const Signature& signature = lang.symbolinfo(symbol_id).signature();
		std::vector<object_id> point;
		assert(var_data["point"].Size() == signature.size());
		for (unsigned j = 0; j < var_data["point"].Size(); ++j) {
			point.push_back(make_object(get_type_id(signature[j]), var_data["point"][j].GetInt()));
		}

		variableDataToId.insert(std::make_pair(std::make_pair(symbol_id, point),  id));
		variableIdToData.push_back(std::make_pair(symbol_id, point));
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
// 		std::vector<VariableIdx> variables;
// 		const auto& list = data[i][5];
// 		for (unsigned j = 0; j < list.Size(); ++j) {
// 			variables.push_back(list[j][0].GetInt());
// 		}

		bool is_static = data[i][6].GetBool();
        bool has_unbounded_arity = data[i][7].GetBool();
		_functionData.push_back(SymbolData(type, domain, codomain, is_static, has_unbounded_arity));
	}
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

bool ProblemInfo::isBoundedType(TypeIdx type) const {
	return fstrips::LanguageInfo::instance().typeinfo(type).bounded();
}

type_id ProblemInfo::
get_type_id(const std::string& fstype) const {
	const fstrips::LanguageInfo& lang = fstrips::LanguageInfo::instance();
	return lang.typeinfo(lang.get_fstype_id(fstype)).get_type_id();
}

type_id ProblemInfo::
get_type_id(TypeIdx fstype) const { return fstrips::LanguageInfo::instance().typeinfo(fstype).get_type_id(); }

std::vector<type_id>
ProblemInfo::get_type_ids( const Signature& sign ) const {

	std::vector<type_id> sym_signature_types;
	for ( TypeIdx t : sign ) {
		sym_signature_types.push_back(get_type_id(t));
	}
	return sym_signature_types;
}

const std::vector<object_id>& ProblemInfo::
getTypeObjects(TypeIdx fstype) const { return fstrips::LanguageInfo::instance().type_objects(fstype); }

TypeIdx ProblemInfo::
getTypeId(const std::string& type_name) const { return fstrips::LanguageInfo::instance().get_fstype_id(type_name); }

const std::string& ProblemInfo::
getTypename(TypeIdx fstype) const { return fstrips::LanguageInfo::instance().typeinfo(fstype).name(); }


void ProblemInfo::
checkValueIsValid(VariableIdx variable, const object_id& object) const {
	TypeIdx type = getVariableType(variable);
	fstrips::LanguageInfo::instance().check_valid_object(object, type);
}

const std::pair<int,int> ProblemInfo::getTypeBounds(TypeIdx type) const {
	return fstrips::LanguageInfo::instance().typeinfo(type).bounds<int>();
}

unsigned ProblemInfo::
num_objects() const { return fstrips::LanguageInfo::instance().num_objects(); }

std::string ProblemInfo::
object_name(const object_id& object) const { return fstrips::LanguageInfo::instance().get_object_name(object); }

const object_id ProblemInfo::get_object_id(const std::string& name) const {return fstrips::LanguageInfo::instance().get_object_id(name);}



} // namespaces
