
#pragma once

#include <languages/fstrips/language.hxx>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

namespace fs0 { namespace language { namespace fstrips {

class Loader {
public:
	//! Parse an (unprocessed) term from a JSON node
	static TermSchema::cptr parseTerm(const pt::ptree& tree);
	
	//! Parse an atomic formula from a JSON node
	static AtomicFormulaSchema::cptr parseAtomicFormula(const pt::ptree& tree);
	
	
	
};

} } } // namespaces
