
#include <languages/fstrips/loader.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/builtin.hxx>
#include <constraints/registry.hxx>


namespace fs0 { namespace language { namespace fstrips {


const Formula* Loader::parseFormula(const rapidjson::Value& tree, const ProblemInfo& info) {
	// As of now we only accept either conjunctions of atoms or existentially quantified conjunctions
	std::string formula_type = tree["type"].GetString();
	
	
	if (formula_type == "conjunction") {
		std::vector<const AtomicFormula*> list;
		const rapidjson::Value& children = tree["children"];
		for (unsigned i = 0; i < children.Size(); ++i) {
			const AtomicFormula* atomic = dynamic_cast<const AtomicFormula*>(parseFormula(children[i], info));
			if (!atomic) {
				throw std::runtime_error("Only conjunctions of atoms supported so far");
			}
			list.push_back(atomic);
		}
		return new Conjunction(list);
	
		
	} else if (formula_type == "existential" || formula_type == "forall") {
		auto subformula = parseFormula(tree["subformula"], info);
		std::vector<const BoundVariable*> variables = parseVariables(tree["variables"], info);
		
		if (formula_type == "existential") {
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
				IntConstant* value = negated ? new IntConstant(0) : new IntConstant(1);
				
				subterms = {NestedTerm::create(symbol, subterms), value};
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
		return new Constant(tree["value"].GetInt());
	} else if (term_type == "int_constant") {
		return new IntConstant(tree["value"].GetInt());
	} else if (term_type == "parameter") {
		return new BoundVariable(tree["position"].GetInt(), info.getTypeId(tree["typename"].GetString()));
	} else if (term_type == "function") {
		std::string symbol = tree["symbol"].GetString();
		std::vector<const Term*> children = parseTermList(tree["children"], info);
		
		try { return LogicalComponentRegistry::instance().instantiate_term(symbol, children); }
		catch(const std::runtime_error& e) {}
		
		return NestedTerm::create(symbol, children);
	} else throw std::runtime_error("Unknown node type " + term_type);
}

std::vector<const BoundVariable*> Loader::parseVariables(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<const BoundVariable*> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		const rapidjson::Value& node = tree[i];
		unsigned id = node[0].GetUint();
// 		std::string name = node[1].GetString();
		std::string type_name = node[2].GetString();
		TypeIdx type = info.getTypeId(type_name);
		list.push_back(new BoundVariable(id, type));
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
