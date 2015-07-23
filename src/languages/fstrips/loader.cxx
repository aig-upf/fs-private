
#include <languages/fstrips/loader.hxx>


namespace pt = boost::property_tree;

namespace fs0 { namespace language { namespace fstrips {

AtomicFormulaSchema::cptr Loader::parseAtomicFormula(const pt::ptree& tree) {
	std::string term_type = tree.get<std::string>("type");
	
	if (term_type == "atom") {
		std::string symbol = tree.get<std::string>("symbol");
		std::vector<TermSchema::cptr> subterms = parseTermList(tree.get_child("subterms"));
		if (subterms.size() != 2) std::runtime_error("Only binary atoms accepted so far");
		return AtomicFormulaSchema::create(symbol, subterms[0], subterms[1]);

	} else throw std::runtime_error("Unknown node type " + term_type);
}

TermSchema::cptr Loader::parseTerm(const pt::ptree& tree) {
	std::string term_type = tree.get<std::string>("type");
	
	if (term_type == "constant") {
		return new ConstantSchema(tree.get<int>("value"));
	} else if (term_type == "parameter") {
		return new ActionSchemaParameter(tree.get<unsigned>("position"));
	} else if (term_type == "nested") {
		unsigned symbol_id = tree.get<unsigned>("symbol");
		std::vector<TermSchema::cptr> subterms = parseTermList(tree.get_child("subterms"));
		return new NestedTermSchema(symbol_id, subterms);
		
	} else throw std::runtime_error("Unknown node type " + term_type);
}

std::vector<TermSchema::cptr> Loader::parseTermList(const pt::ptree& tree) {
	std::vector<TermSchema::cptr> list;
	for (auto& it:tree) {
		list.push_back(parseTerm(it.second));
	}
	return list;
}


} } } // namespaces
