
#pragma once

#include <languages/fstrips/schemata.hxx>
#include <lib/rapidjson/document.h>

namespace fs0 {
	class ProblemInfo;
}

namespace fs0 { namespace language { namespace fstrips {

class Loader {
public:
	//! Parse an atomic formula from a JSON node
	static AtomicFormulaSchema::cptr parseAtomicFormula(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse a list of atomic formulae
	static std::vector<AtomicFormulaSchema::cptr> parseAtomicFormulaList(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse an (unprocessed) term from a JSON node
	static TermSchema::cptr parseTerm(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse a list of terms
	static std::vector<TermSchema::cptr> parseTermList(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse an atomic formula from a JSON node
	static ActionEffectSchema::cptr parseAtomicEffect(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse a list of atomic formulae from a JSON node
	static std::vector<ActionEffectSchema::cptr> parseAtomicEffectList(const rapidjson::Value& tree, const ProblemInfo& info);
	
};

} } } // namespaces
