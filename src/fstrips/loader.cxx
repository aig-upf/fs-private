
#include <lapkt/tools/logging.hxx>

#include <fstrips/language.hxx>
#include <fstrips/loader.hxx>
#include <fstrips/language_info.hxx>
#include <fstrips/operations.hxx>
#include <utils/loader.hxx>

#include <languages/fstrips/builtin.hxx>
#include <constraints/registry.hxx>


namespace fs0 { namespace fstrips {



std::vector<const LogicalVariable*> Loader::
parseVariables(const rapidjson::Value& tree, const LanguageInfo& lang) {
	std::vector<const LogicalVariable*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		const rapidjson::Value& node = tree[i];
		unsigned id = node[0].GetUint();
 		std::string name = node[1].GetString();
		std::string type_name = node[2].GetString();
		TypeIdx type = lang.get_fstype_id(type_name);
		list.push_back(new LogicalVariable(id, name, type));
	}
	return list;
}

//! Parse a list of terms
std::vector<const Term*> _parseTermList(const rapidjson::Value& tree, const LanguageInfo& lang) {
	std::vector<const Term*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(Loader::parseTerm(tree[i], lang));
	}
	return list;
}


const Formula* Loader::parseFormula(const rapidjson::Value& tree, const LanguageInfo& lang) {
	std::string formula_type = tree["type"].GetString();

	if (formula_type == "and" || formula_type == "or" || formula_type == "not") {
		std::vector<const Formula*> subformulae;
		const rapidjson::Value& children = tree["children"];
		for (unsigned i = 0; i < children.Size(); ++i) {
			subformulae.push_back(parseFormula(children[i], lang));
		}

		return new OpenFormula(to_connective(formula_type), subformulae);


	} else if (formula_type == "exists" || formula_type == "forall") {
		std::vector<const LogicalVariable*> variables = parseVariables(tree["variables"], lang);
		auto subformula = parseFormula(tree["subformula"], lang);

		return new QuantifiedFormula(to_quantifier(formula_type), variables, subformula);

	} else if (formula_type == "atom") {
		std::string symbol = tree["symbol"].GetString();
		unsigned symbol_id = lang.get_symbol_id(symbol);
		bool negated = tree["negated"].GetBool(); // TODO NEGATED SHOULDN'T BE HERE, BUT RATHER A NEGATION.
		std::vector<const Term*> subterms = _parseTermList(tree["children"], lang);

		return new AtomicFormula(symbol_id, subterms);

	} else if (formula_type == "tautology") {
		return new Tautology;
	} else if (formula_type == "contradiction") {
		return new Contradiction;
	}

	throw std::runtime_error("Unknown formula type \"" + formula_type + "\"");
}


const Term* Loader::parseTerm(const rapidjson::Value& tree, const LanguageInfo& lang) {
	std::string term_type = tree["type"].GetString();

	if (term_type == "constant") {
		TypeIdx fstype = lang.get_fstype_id(tree["fstype"].GetString());
		object_id object = make_object(lang.get_type_id(fstype), tree["value"].GetInt());
		return new Constant(object, fstype);

	} else if (term_type == "variable") {
		return new LogicalVariable(tree["position"].GetInt(), tree["name"].GetString(), lang.get_fstype_id(tree["typename"].GetString()));

	} else if (term_type == "functional") {
		std::string symbol = tree["symbol"].GetString();
		unsigned symbol_id = lang.get_symbol_id(symbol);
		std::vector<const Term*> children = _parseTermList(tree["children"], lang);
		return new FunctionalTerm(symbol_id, children);
	}

	throw std::runtime_error("Unknown term type " + term_type);
}





const ActionEffect* Loader::parseEffect(const rapidjson::Value& tree, const LanguageInfo& lang) {
	const std::string effect_type = tree["type"].GetString();
	const Formula* condition = parseFormula(tree["condition"], lang);

	if (effect_type == "functional") {
		const FunctionalTerm* lhs = dynamic_cast<const FunctionalTerm*>(parseTerm(tree["lhs"], lang));
		if (!lhs) {
			throw std::runtime_error("Invalid LHS of a functional effect");
		}

		return new FunctionalEffect(lhs, parseTerm(tree["rhs"], lang), condition);

	} else if (effect_type == "add" || effect_type == "del") {
		AtomicEffect::Type type = AtomicEffect::to_type(effect_type);
		const AtomicFormula* atom = dynamic_cast<const AtomicFormula*>(parseFormula(tree["lhs"], lang));
		if (!atom) {
			throw std::runtime_error("Invalid LHS of an atomic effect");
		}

		return new AtomicEffect(atom, type, condition);

	}

	throw std::runtime_error("Unknown effect type " + effect_type);
}

std::vector<const ActionEffect*> Loader::parseEffectList(const rapidjson::Value& tree, const LanguageInfo& lang) {
	std::vector<const ActionEffect*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseEffect(tree[i], lang));
	}
	return list;
}


const ActionSchema*
Loader::parseActionSchema(const rapidjson::Value& node, unsigned id, const LanguageInfo& lang, bool load_effects) {
	const std::string& name = node["name"].GetString();
	const Signature signature = fs0::Loader::parseNumberList<unsigned>(node["signature"]);
	const std::vector<std::string> parameters = fs0::Loader::parseStringList(node["parameters"]);

	const Formula* precondition = parseFormula(node["conditions"], lang);
	std::vector<const ActionEffect*> effects;

	if (load_effects) {
		effects = parseEffectList(node["effects"], lang);
	}

	ActionSchema* schema = new ActionSchema(id, name, signature, parameters, precondition, effects);
	if (has_empty_parameter(*schema)) {
		LPT_INFO("cout", "Schema \"" << schema->getName() << "\" discarded because of empty parameter type.");
		delete schema;
		return nullptr;
	}

	return schema;
	// We perform a first binding on the action schema so that state variables, etc. get consolidated, but the parameters remain the same
	// This is possibly not optimal, since for some configurations we might be duplicating efforts, but ATM we are happy with it
// 	return ActionGrounder::process_action_data(adata, lang, load_effects);
}


//! Load all the function-related data
void _loadSymbolIndex(const rapidjson::Value& data, LanguageInfo& lang) {
	// Symbol data is stored as: # <symbol_id, symbol_name, symbol_type, <function_domain>, function_codomain, state_variables, static?>
	for (unsigned i = 0; i < data.Size(); ++i) {
		unsigned expected_id = data[i][0].GetInt();
		_unused(expected_id);
		
		std::string name(data[i][1].GetString());

		// Parse the symbol type: function or predicate
		const std::string symbol_type = data[i][2].GetString();
		assert (symbol_type == "function" || symbol_type == "predicate");
		symbol_t type = (symbol_type == "function") ? symbol_t::Function : symbol_t::Predicate;


		// Parse the symbol signature
		Signature signature;
		const auto& domains = data[i][3];
		for (unsigned j = 0; j < domains.Size(); ++j) {
			signature.push_back(lang.get_fstype_id(domains[j].GetString()));
		}

		if (type == symbol_t::Function) {
			// Parse the codomain ID
			signature.push_back(lang.get_fstype_id(data[i][4].GetString()));
		}

		bool static_ = data[i][6].GetBool();
		/*
        bool has_unbounded_arity = data[i][7].GetBool();
		_functionData.push_back(SymbolData(type, domain, codomain, variables, is_static, has_unbounded_arity));
		*/

		symbol_id id = lang.add_symbol(name, type, signature, static_);
		_unused(id);
		assert(expected_id == id); // Check values are decoded in the proper order
	}
}


//! Load the names of the state variables
// 	void loadVariableIndex(const rapidjson::Value& data, LanguageInfo& lang);


//! Load the names of the problem objects
void _loadObjectIndex(const rapidjson::Value& data, LanguageInfo& lang) {
	for (unsigned i = 0; i < data.Size(); ++i) {
		unsigned expected_id = static_cast<unsigned>(data[i]["id"].GetInt());
		_unused(expected_id);
		
		const std::string& name = data[i]["name"].GetString();
		const std::string& fstype = data[i]["type"].GetString();

		object_id id = lang.add_object(name, lang.get_fstype_id(fstype));
		_unused(id);
		assert((int)expected_id == (int)id); // Check values are decoded in the proper order
	}
}


//! Load all type-related info.
void _loadTypeIndex(const rapidjson::Value& data, LanguageInfo& lang) {
	for (unsigned i = 0; i < data.Size(); ++i) {
		const auto& node = data[i];


		TypeIdx expected_id = node["id"].GetInt();
		_unused(expected_id);
		std::string fstype = node["fstype"].GetString();
		std::string domain_type = node["domain_type"].GetString();
		std::string type_id_str = node["type_id"].GetString();

		TypeIdx tid;
		_unused(tid);
		
		if (domain_type == "unbounded") {
			tid = lang.add_fstype( fstype, from_string(type_id_str));
			assert( tid == expected_id );
			
		} else if (domain_type == "interval") {
			int lower = node["interval"][0].GetInt();
			int upper = node["interval"][1].GetInt();

			tid = lang.add_fstype(fstype, from_string(type_id_str), make_range(lower, upper));
			assert(tid == expected_id);
			
		} else if (domain_type == "set") {
			tid = lang.add_fstype(fstype, from_string(type_id_str));
			assert(tid == expected_id);

			for (unsigned j = 0; j < node["set"].Size(); ++j) {
				int value = boost::lexical_cast<int>(node["set"][j].GetString());
				lang.bind_object_to_type(lang.get_fstype_id(fstype), make_object(type_id::object_t, value));
			}
		}
	}
}

void LanguageJsonLoader::
loadLanguageInfo(const rapidjson::Document& data) {
	LanguageInfo* lang = new LanguageInfo();

	LPT_INFO("cout", "Loading FS types from JSON file");
	_loadTypeIndex(data["types"], *lang); // Order matters

	LPT_INFO("cout", "Loading language objects from JSON file");
	_loadObjectIndex(data["objects"], *lang);

	LPT_INFO("cout", "Loading language symbols from JSON file");
	_loadSymbolIndex(data["symbols"], *lang);

	LanguageInfo::instance(lang);
}

} } // namespaces
