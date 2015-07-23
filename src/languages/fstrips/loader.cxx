
#include <languages/fstrips/loader.hxx>


namespace pt = boost::property_tree;

namespace fs0 { namespace language { namespace fstrips {

TermSchema::cptr Loader::parseTerm(const pt::ptree& tree) {
	std::string term_type = tree.get<std::string>("type");
	
	if (term_type == "constant") {
		return new ConstantSchema(tree.get<int>("value"));
	} else if (term_type == "parameter") {
		return new ActionSchemaParameter(tree.get<unsigned>("position"));
	} else if (term_type == "nested") {
		unsigned symbol_id = tree.get<unsigned>("symbol");
		std::vector<TermSchema::cptr> subterms;
		for (auto& it:tree.get_child("args")) {
			subterms.push_back(parseTerm(it.second));
		}
		return new NestedTermSchema(symbol_id, subterms);
		
	} else {
		throw std::runtime_error("Unknown term type " + term_type);
	}
}


AtomicFormulaSchema::cptr Loader::parseAtomicFormula(const pt::ptree& tree) {
	
}

} } } // namespaces
