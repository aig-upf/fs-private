
#include <languages/fstrips/loader.hxx>
#include "builtin.hxx"
#include <problem.hxx>
#include <constraints/registry.hxx>


namespace fs0 { namespace language { namespace fstrips {

AtomicFormula::cptr Loader::parseAtomicFormula(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::string term_type = tree["type"].GetString();
	
	if (term_type == "atom") {
		std::string symbol = tree["symbol"].GetString();
		std::vector<Term::cptr> subterms = parseTermList(tree["subterms"], info);
		return LogicalComponentRegistry::instance().instantiate_formula(symbol, subterms);
	}
	else throw std::runtime_error("Unknown node type " + term_type);
}

Formula::cptr Loader::parseFormula(const rapidjson::Value& tree, const ProblemInfo& info) {
	// As of now we only accept either conjunctions of atoms or existentially quantified conjunctions
	std::string formula_type = tree["type"].GetString();
	
	
	if (formula_type == "conjunction") {
		std::vector<AtomicFormula::cptr> list;
		const rapidjson::Value& elements = tree["elements"];
		for (unsigned i = 0; i < elements.Size(); ++i) {
			list.push_back(parseAtomicFormula(elements[i], info));
		}
		return new Conjunction(list);
	
		
	} else if (formula_type == "existential") {
		auto subformula = parseFormula(tree["subformula"], info);
		auto subformula_conjunction = dynamic_cast<Conjunction::cptr>(subformula);
		if (!subformula_conjunction) {
			throw std::runtime_error("Only existentially quantified conjunctions are supported so far");
		}
		std::vector<BoundVariable> variables = parseVariables(tree["variables"], info);
		return new ExistentiallyQuantifiedFormula(variables, subformula_conjunction);
	
		
	} else if (formula_type == "tautology") {
		return new Tautology;
	} else if (formula_type == "contradiction") {
		return new Contradiction;
	}
	
	throw std::runtime_error("Unknown formula type " + formula_type);
}

std::vector<BoundVariable> Loader::parseVariables(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<BoundVariable> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		const rapidjson::Value& node = tree[i];
		unsigned id = node[0].GetUint();
// 		std::string name = node[1].GetString();
		std::string type_name = node[2].GetString();
		TypeIdx type = info.getTypeId(type_name);
		list.push_back(BoundVariable(id, type));
	}
	return list;
}

Term::cptr Loader::parseTerm(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::string term_type = tree["type"].GetString();
	
	if (term_type == "constant") {
		return new Constant(tree["value"].GetInt());
	} else if (term_type == "int_constant") {
		return new IntConstant(tree["value"].GetInt());
	} else if (term_type == "parameter") {
		return new BoundVariable(tree["position"].GetInt(), info.getTypeId(tree["typename"].GetString()));
	} else if (term_type == "function") {
		std::string symbol = tree["symbol"].GetString();
		std::vector<Term::cptr> subterms = parseTermList(tree["subterms"], info);
		return NestedTerm::create(symbol, subterms);
	} else throw std::runtime_error("Unknown node type " + term_type);
}

std::vector<Term::cptr> Loader::parseTermList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<Term::cptr> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseTerm(tree[i], info));
	}
	return list;
}

ActionEffect::cptr Loader::parseEffect(const rapidjson::Value& tree, const ProblemInfo& info) {
	assert(tree.Size() == 2);
	return new ActionEffect(parseTerm(tree[0], info), parseTerm(tree[1], info));
}

std::vector<ActionEffect::cptr> Loader::parseEffectList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<ActionEffect::cptr> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseEffect(tree[i], info));
	}
	return list;
}



} } } // namespaces
