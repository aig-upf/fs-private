
#include <fs/core/constraints/registry.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 {

std::unique_ptr<LogicalComponentRegistry> LogicalComponentRegistry::_instance = nullptr;

void LogicalComponentRegistry::registerLogicalElementCreators() {
	// Standard relational formulae
	addFormulaCreator("=",  [](const std::vector<const fs::Term*>& subterms){ return new fs::EQAtomicFormula(subterms); });
	addFormulaCreator("!=", [](const std::vector<const fs::Term*>& subterms){ return new fs::NEQAtomicFormula(subterms); });
	addFormulaCreator("<",  [](const std::vector<const fs::Term*>& subterms){ return new fs::LTAtomicFormula(subterms); });
	addFormulaCreator("<=", [](const std::vector<const fs::Term*>& subterms){ return new fs::LEQAtomicFormula(subterms); });
	addFormulaCreator(">",  [](const std::vector<const fs::Term*>& subterms){ return new fs::GTAtomicFormula(subterms); });
	addFormulaCreator(">=", [](const std::vector<const fs::Term*>& subterms){ return new fs::GEQAtomicFormula(subterms); });

	// Register the builtin global constraints
	addFormulaCreator("@alldiff", [](const std::vector<const fs::Term*>& subterms){ return new fs::AlldiffFormula(subterms); });
	addFormulaCreator("@sum",     [](const std::vector<const fs::Term*>& subterms){ return new fs::SumFormula(subterms); });
	addFormulaCreator("@nvalues", [](const std::vector<const fs::Term*>& subterms){ return new fs::NValuesFormula(subterms); });
}



LogicalComponentRegistry::LogicalComponentRegistry() {
	// TODO - The actual initialization should probably be moved somewhere else
	registerLogicalElementCreators();
}

LogicalComponentRegistry::~LogicalComponentRegistry() = default;

void LogicalComponentRegistry::addFormulaCreator(const std::string& symbol, const FormulaCreator& creator) {
	auto res = _formula_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw std::runtime_error("Duplicate registration of formula creator for symbol " + symbol);
}

void LogicalComponentRegistry::addTermCreator(const std::string& symbol, const TermCreator& creator) {
	auto res = _term_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw std::runtime_error("Duplicate registration of term creator for symbol " + symbol);
}

void LogicalComponentRegistry::addEffectCreator(const std::string& symbol, const EffectCreator& creator) {
	auto res = _effect_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw std::runtime_error("Duplicate registration of effect creator for symbol " + symbol);
}

/*
void LogicalComponentRegistry::add(const std::type_info& type, const DirectFormulaTranslator& translator) {
	auto res = _direct_formula_translators.insert(std::make_pair(std::type_index(type), translator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of formula translator for class " + print::type_info_name(type));
}

void LogicalComponentRegistry::add(const std::type_info& type, const EffectTranslator* translator) {
	auto res = _direct_effect_translators.insert(std::make_pair(std::type_index(type), translator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of effect translator for class " + print::type_info_name(type));
}
*/


const fs::AtomicFormula* LogicalComponentRegistry::instantiate_formula(const std::string& symbol, const std::vector<const fs::Term*>& subterms) const {
	auto it = _formula_creators.find(symbol);
	if (it == _formula_creators.end()) throw std::runtime_error("An externally defined symbol '" + symbol + "' is being used without having registered a suitable term/formula creator for it");
	return it->second(subterms);
}

const fs::Term* LogicalComponentRegistry::instantiate_term(const std::string& symbol, const std::vector<const fs::Term*>& subterms) const {
	auto it = _term_creators.find(symbol);
	if (it == _term_creators.end()) throw std::runtime_error("An externally defined symbol '" + symbol + "' is being used without having registered a suitable term/formula creator for it");
	return it->second(subterms);
}

fs::ProceduralEffect* LogicalComponentRegistry::instantiate_effect(const std::string& symbol) const {
	auto it = _effect_creators.find(symbol);
	if (it == _effect_creators.end()) throw std::runtime_error("An externally defined effect with symbol '" + symbol + "' is being used without having registered a suitable effect creator for it");
	return it->second();
}

} // namespaces
