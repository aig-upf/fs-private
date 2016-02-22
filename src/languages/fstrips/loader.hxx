
#pragma once

#include <lib/rapidjson/document.h>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/effects.hxx>

namespace fs0 {
	class ProblemInfo;
}

namespace fs0 { namespace language { namespace fstrips {

class Loader {
public:
	//! Parse an atomic formula from a JSON node
	static AtomicFormula::cptr parseAtomicFormula(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse a list of atomic formulae
	static Formula::cptr parseFormula(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse an (unprocessed) term from a JSON node
	static Term::cptr parseTerm(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse a list of quantified variables 
	static std::vector<BoundVariable> parseVariables(const rapidjson::Value& tree, const ProblemInfo& info);

	
	//! Parse a list of terms
	static std::vector<Term::cptr> parseTermList(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse an atomic formula from a JSON node
	static ActionEffect::cptr parseEffect(const rapidjson::Value& tree, const fs0::ProblemInfo& info);
	
	//! Parse a list of atomic formulae from a JSON node
	static std::vector<ActionEffect::cptr> parseEffectList(const rapidjson::Value& tree, const ProblemInfo& info);
};

} } } // namespaces
