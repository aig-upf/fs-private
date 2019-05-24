
#pragma once

#include <vector>
#include <rapidjson/document.h>

namespace fs0 { namespace fstrips {

class LanguageInfo;
class Term;
class ActionEffect;
class Formula;
class AtomicFormula;
class ActionSchema;
class LogicalVariable;

class Loader {
public:
	//! Parse a formula from a JSON node
	static const Formula* parseFormula(const rapidjson::Value& tree, const LanguageInfo& lang);

	//! Parse an (unprocessed) term from a JSON node
	static const Term* parseTerm(const rapidjson::Value& tree, const LanguageInfo& lang);

	//! Parse an atomic formula from a JSON node
	static const ActionEffect* parseEffect(const rapidjson::Value& tree, const LanguageInfo& lang);

	//! Parse a list of atomic formulae from a JSON node
	static std::vector<const ActionEffect*> parseEffectList(const rapidjson::Value& tree, const LanguageInfo& lang);

	//!
	static const ActionSchema* parseActionSchema(const rapidjson::Value& data, unsigned id, const LanguageInfo& lang, bool load_effects);

	static std::vector<const LogicalVariable*> parseVariables(const rapidjson::Value& tree, const LanguageInfo& lang);
};

class LanguageJsonLoader {
public:
	static void loadLanguageInfo(const rapidjson::Document& data);
};

} } // namespaces
