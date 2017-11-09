
#include <fs/core/constraints/registry.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
// #include <fs/core/constraints//direct/alldiff_constraint.hxx>
// #include <fs/core/constraints//direct/sum_constraint.hxx>
#include <fs/core/utils//printers/printers.hxx>
#include <fs/core/utils//printers/helper.hxx>
// #include <fs/core/constraints//direct/constraint.hxx>
// #include <fs/core/constraints//direct/translators/effects.hxx>
#include <fs/core/constraints/gecode/translators/component_translator.hxx>
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
	addFormulaCreator("@nvalues",     [](const std::vector<const fs::Term*>& subterms){ return new fs::NValuesFormula(subterms); });
}



/*
void LogicalComponentRegistry::registerDirectTranslators() {
	add(typeid(fs::Constant),           new ConstantRhsTranslator());
	add(typeid(fs::StateVariable),      new StateVariableRhsTranslator());
	add(typeid(fs::AdditionTerm),       new AdditiveTermRhsTranslator());
	add(typeid(fs::SubtractionTerm),    new SubtractiveTermRhsTranslator());
	add(typeid(fs::MultiplicationTerm), new MultiplicativeTermRhsTranslator());

	// builtin global constraints
	add(typeid(fs::AlldiffFormula), [](const fs::AtomicFormula& formula){ return new AlldiffConstraint(fs::ScopeUtils::computeDirectScope(&formula)); });
	add(typeid(fs::SumFormula), [](const fs::AtomicFormula& formula){ return new SumConstraint(fs::ScopeUtils::computeDirectScope(&formula)); });
}
*/


void LogicalComponentRegistry::registerGecodeTranslators() {
	// Register the gecode translators for the basic terms
	add(typeid(fs::Constant), new gecode::ConstantTermTranslator());
	add(typeid(fs::StaticHeadedNestedTerm), new gecode::StaticNestedTermTranslator());
	add(typeid(fs::UserDefinedStaticTerm), new gecode::StaticNestedTermTranslator()); // user-defined terms can be translated with the "parent" static translator
	add(typeid(fs::BoundVariable), new gecode::BoundVariableTermTranslator());

	add(typeid(fs::AdditionTerm), new gecode::AdditionTermTranslator());
	add(typeid(fs::SubtractionTerm), new gecode::SubtractionTermTranslator());
	add(typeid(fs::MultiplicationTerm), new gecode::MultiplicationTermTranslator());



	add(typeid(fs::ExistentiallyQuantifiedFormula), new gecode::ExistentiallyQuantifiedFormulaTranslator());
	add(typeid(fs::Conjunction), new gecode::ConjunctionTranslator());
	add(typeid(fs::RelationalFormula), new gecode::RelationalFormulaTranslator());
	add(typeid(fs::EQAtomicFormula), new gecode::RelationalFormulaTranslator());
	add(typeid(fs::NEQAtomicFormula), new gecode::RelationalFormulaTranslator());
	add(typeid(fs::LTAtomicFormula), new gecode::RelationalFormulaTranslator());
	add(typeid(fs::LEQAtomicFormula), new gecode::RelationalFormulaTranslator());
	add(typeid(fs::GTAtomicFormula), new gecode::RelationalFormulaTranslator());
	add(typeid(fs::GEQAtomicFormula), new gecode::RelationalFormulaTranslator());

	// Gecode translators for the supported global constraints
	add(typeid(fs::AlldiffFormula), new gecode::AlldiffGecodeTranslator());
	add(typeid(fs::SumFormula), new gecode::SumGecodeTranslator());
	add(typeid(fs::NValuesFormula), new gecode::NValuesGecodeTranslator());
}


LogicalComponentRegistry::LogicalComponentRegistry() {
	// TODO - The actual initialization should probably be moved somewhere else
	registerLogicalElementCreators();
// 	registerDirectTranslators();
	registerGecodeTranslators();
}

LogicalComponentRegistry::~LogicalComponentRegistry() {
	 // Delete all the pointers to gecode translators
	for (const auto elem:_gecode_term_translators) delete elem.second;
	for (const auto elem:_gecode_formula_translators) delete elem.second;
// 	for (const auto elem:_direct_effect_translators) delete elem.second;
}

void LogicalComponentRegistry::addFormulaCreator(const std::string& symbol, const FormulaCreator& creator) {
	auto res = _formula_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of formula creator for symbol " + symbol);
}

void LogicalComponentRegistry::addTermCreator(const std::string& symbol, const TermCreator& creator) {
	auto res = _term_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of term creator for symbol " + symbol);
}

void LogicalComponentRegistry::addEffectCreator(const std::string& symbol, const EffectCreator& creator) {
	auto res = _effect_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of effect creator for symbol " + symbol);
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


void LogicalComponentRegistry::add(const std::type_info& type, const gecode::TermTranslator* translator) {
	auto res = _gecode_term_translators.insert(std::make_pair(std::type_index(type), translator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of gecode translator for class " + print::type_info_name(type));
}

void LogicalComponentRegistry::add(const std::type_info& type, const gecode::FormulaTranslator* translator) {
	auto res = _gecode_formula_translators.insert(std::make_pair(std::type_index(type), translator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of gecode translator for class " + print::type_info_name(type));
}

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

/*
DirectConstraint* LogicalComponentRegistry::instantiate_direct_constraint(const fs::AtomicFormula& formula) const {
	auto it = _direct_formula_translators.find(std::type_index(typeid(formula)));
	if (it == _direct_formula_translators.end()) return nullptr;
	return it->second(formula);
}

const EffectTranslator* LogicalComponentRegistry::getDirectEffectTranslator(const fs::Term& term) const {
	auto it = _direct_effect_translators.find(std::type_index(typeid(term)));
	if (it == _direct_effect_translators.end()) return nullptr;
	return it->second;
}
*/

const gecode::TermTranslator* LogicalComponentRegistry::getGecodeTranslator(const fs::Term& term) const {
	auto it = _gecode_term_translators.find(std::type_index(typeid(term)));
	if (it == _gecode_term_translators.end()) {
		throw UnregisteredGecodeTranslator(term);
	}
	return it->second;
}

const gecode::FormulaTranslator* LogicalComponentRegistry::getGecodeTranslator(const fs::AtomicFormula& formula) const {
	auto it = _gecode_formula_translators.find(std::type_index(typeid(formula)));
	if (it == _gecode_formula_translators.end()) {
		throw UnregisteredGecodeTranslator(formula);
	}
	return it->second;
}

template <typename T>
std::string UnregisteredGecodeTranslator::message(const T& element) {
	return printer() << "A Gecode translator is required for element \""<< element << "\", but none was registered";
}

} // namespaces
