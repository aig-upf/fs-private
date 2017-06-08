
#include <languages/fstrips/loader.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/builtin.hxx>
#include <languages/fstrips/axioms.hxx>
#include <constraints/registry.hxx>
#include <problem.hxx>


namespace fs0 { namespace language { namespace fstrips {

//! Factory method to create a nested term of the appropriate type
const Term* _create_nested_term(const std::string& symbol, const std::vector<const Term*>& subterms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	// If the symbol corresponds to an arithmetic term, delegate the creation of the term
	if (ArithmeticTermFactory::isBuiltinTerm(symbol)) return ArithmeticTermFactory::create(symbol, subterms);

	unsigned symbol_id = info.getSymbolId(symbol);	
	const auto& function = info.getSymbolData(symbol_id);
	if (function.isStatic()) {
		return new UserDefinedStaticTerm(symbol_id, subterms);
	} else {
		return new FluentHeadedNestedTerm(symbol_id, subterms);
	}
}

const Formula* Loader::parseFormula(const rapidjson::Value& tree, const ProblemInfo& info) {
	// As of now we only accept either conjunctions of atoms or existentially quantified conjunctions
	std::string formula_type = tree["type"].GetString();
	
	
	if (formula_type == "and" || formula_type == "or") {
		std::vector<const Formula*> list;
		const rapidjson::Value& children = tree["children"];
		for (unsigned i = 0; i < children.Size(); ++i) {
			list.push_back(parseFormula(children[i], info));
		}
		
		if (formula_type == "and") {
			return new Conjunction(list);
		} else {
			return new Disjunction(list);
		}		
	
		
	} else if (formula_type == "exists" || formula_type == "forall") {
		auto subformula = parseFormula(tree["subformula"], info);
		std::vector<const BoundVariable*> variables = parseVariables(tree["variables"], info);
		
		if (formula_type == "exists") {
			return new ExistentiallyQuantifiedFormula(variables, subformula);
		} else {
			return new UniversallyQuantifiedFormula(variables, subformula);
		}
	
	} else if (formula_type == "atom") {
		std::string symbol = tree["symbol"].GetString();
		bool negated = tree["negated"].GetBool();
		std::vector<const Term*> subterms = parseTermList(tree["children"], info);
		
		// HACK - WONT WORK FOR NEGATED FORMULAS
		try { return LogicalComponentRegistry::instance().instantiate_formula(symbol, subterms); }
		catch(const std::runtime_error& e) {}
		
		// TODO - This is a temporary hack to parse predicates 'p(x)' as if they were
		// equality predicates 'p(x) = 1' with 'p' being a binary function.
		try {
			unsigned symbol_id = info.getSymbolId(symbol);
			if (info.isPredicate(symbol_id)) {
				// 
				IntConstant* value = negated ? new IntConstant(make_obj<int>(0)) : new IntConstant(make_obj<int>(1));
				
				subterms = {_create_nested_term(symbol, subterms), value};
				symbol = "=";
			}
		} catch(std::out_of_range& ex) {} // The symbol might be built-in, and thus not registered.
		
		return LogicalComponentRegistry::instance().instantiate_formula(symbol, subterms);


	} else if (formula_type == "tautology") {
		return new Tautology;
	} else if (formula_type == "contradiction") {
		return new Contradiction;
	}
	
	throw std::runtime_error("Unknown formula type \"" + formula_type + "\"");
}


const Term* Loader::parseTerm(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::string term_type = tree["type"].GetString();
	
	if (term_type == "constant") {
		return new Constant(make_obj(tree["value"].GetInt()));
	} else if (term_type == "int_constant") {
		return new IntConstant(make_obj(tree["value"].GetInt()));
	} else if (term_type == "variable") {
		return new BoundVariable(tree["position"].GetInt(), tree["name"].GetString(), info.getTypeId(tree["typename"].GetString()));
	} else if (term_type == "functional") {
		std::string symbol = tree["symbol"].GetString();
		std::vector<const Term*> children = parseTermList(tree["children"], info);
		
		try { return LogicalComponentRegistry::instance().instantiate_term(symbol, children); }
		catch(const std::runtime_error& e) {}
		
		return _create_nested_term(symbol, children);
	} else throw std::runtime_error("Unknown node type " + term_type);
}

std::vector<const BoundVariable*> Loader::parseVariables(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<const BoundVariable*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		const rapidjson::Value& node = tree[i];
		unsigned id = node[0].GetUint();
		std::string name = node[1].GetString();
		std::string type_name = node[2].GetString();
		TypeIdx type = info.getTypeId(type_name);
		list.push_back(new BoundVariable(id, name, type));
	}
	return list;
}


std::vector<const Term*> Loader::parseTermList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<const Term*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseTerm(tree[i], info));
	}
	return list;
}

const ActionEffect* Loader::parseEffect(const rapidjson::Value& tree, const ProblemInfo& info) {
	const std::string effect_type = tree["type"].GetString();
	return new ActionEffect(parseTerm(tree["lhs"], info), parseTerm(tree["rhs"], info), parseFormula(tree["condition"], info));
}

std::vector<const ActionEffect*> Loader::parseEffectList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<const ActionEffect*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseEffect(tree[i], info));
	}
	return list;
}


} } } // namespaces
