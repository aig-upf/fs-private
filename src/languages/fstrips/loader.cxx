
#include <languages/fstrips/loader.hxx>
#include <problem.hxx>


namespace fs0 { namespace language { namespace fstrips {

AtomicFormulaSchema::cptr Loader::parseAtomicFormula(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::string term_type = tree["type"].GetString();
	
	if (term_type == "atom") {
		std::string symbol = tree["symbol"].GetString();
		std::vector<TermSchema::cptr> subterms = parseTermList(tree["subterms"], info);
		if (subterms.size() != 2) std::runtime_error("Only binary atoms accepted so far");
		return AtomicFormulaSchema::create(symbol, subterms[0], subterms[1]);

	} else throw std::runtime_error("Unknown node type " + term_type);
}

std::vector<AtomicFormulaSchema::cptr> Loader::parseAtomicFormulaList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<AtomicFormulaSchema::cptr> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseAtomicFormula(tree[i], info));
	}
	return list;
}

TermSchema::cptr Loader::parseTerm(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::string term_type = tree["type"].GetString();
	
	if (term_type == "constant") {
		return new ConstantSchema(tree["value"].GetInt());
	} else if (term_type == "parameter") {
		return new ActionSchemaParameter(tree["position"].GetInt());
	} else if (term_type == "nested") {
		unsigned symbol_id = info.getFunctionId(tree["symbol"].GetString());
		std::vector<TermSchema::cptr> subterms = parseTermList(tree["subterms"], info);
		return new NestedTermSchema(symbol_id, subterms);
		
	} else throw std::runtime_error("Unknown node type " + term_type);
}

std::vector<TermSchema::cptr> Loader::parseTermList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<TermSchema::cptr> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseTerm(tree[i], info));
	}
	return list;
}

ActionEffectSchema::cptr Loader::parseAtomicEffect(const rapidjson::Value& tree, const ProblemInfo& info) {
	assert(tree.Size() == 2);
	return new ActionEffectSchema(parseTerm(tree[0], info), parseTerm(tree[1], info));
}

std::vector<ActionEffectSchema::cptr> Loader::parseAtomicEffectList(const rapidjson::Value& tree, const ProblemInfo& info) {
	std::vector<ActionEffectSchema::cptr> list;
	for (unsigned i = 0; i < tree.Size(); ++i) {
		list.push_back(parseAtomicEffect(tree[i], info));
	}
	return list;
}



} } } // namespaces
