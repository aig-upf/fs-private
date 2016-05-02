
#pragma once

#include <vector>
#include <lib/rapidjson/document.h>

namespace fs0 {
	class ProblemInfo;
}

namespace fs0 { namespace language { namespace fstrips {

class Term;
class ActionEffect;
class Formula;
class AtomicFormula;
class BoundVariable;

class Loader {
public:
	//! Parse a formula from a JSON node
	static const Formula* parseFormula(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse an (unprocessed) term from a JSON node
	static const Term* parseTerm(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse a list of quantified variables 
	static std::vector<const BoundVariable*> parseVariables(const rapidjson::Value& tree, const ProblemInfo& info);

	
	//! Parse a list of terms
	static std::vector<const Term*> parseTermList(const rapidjson::Value& tree, const ProblemInfo& info);
	
	//! Parse an atomic formula from a JSON node
	static const ActionEffect* parseEffect(const rapidjson::Value& tree, const fs0::ProblemInfo& info);
	
	//! Parse a list of atomic formulae from a JSON node
	static std::vector<const ActionEffect*> parseEffectList(const rapidjson::Value& tree, const ProblemInfo& info);
};

} } } // namespaces
