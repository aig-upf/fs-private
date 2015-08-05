
#include <constraints/registry.hxx>
#include <problem.hxx>
#include <languages/fstrips/builtin.hxx>
#include <constraints/direct/alldiff_constraint.hxx>
#include <constraints/direct/sum_constraint.hxx>


namespace fs0 {

LogicalComponentRegistry& LogicalComponentRegistry::instance() {
	static LogicalComponentRegistry theInstance;
	return theInstance;
}

LogicalComponentRegistry::LogicalComponentRegistry() {
	// TODO - The actual initialization should probably be moved somewhere else
	// Register the gecode translators for the basic terms
	add(typeid(fs::Constant), new gecode::ConstantTermTranslator());
	add(typeid(fs::StateVariable), new gecode::StateVariableTermTranslator());
	add(typeid(fs::StaticHeadedNestedTerm), new gecode::StaticNestedTermTranslator());
	add(typeid(fs::FluentHeadedNestedTerm), new gecode::FluentNestedTermTranslator());
	
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
	
	
	// Standard relational formulae
	add("=",  [](const std::vector<fs::Term::cptr>& subterms){ return new fs::EQAtomicFormula(subterms); });
	add("!=", [](const std::vector<fs::Term::cptr>& subterms){ return new fs::NEQAtomicFormula(subterms); });
	add("<",  [](const std::vector<fs::Term::cptr>& subterms){ return new fs::LTAtomicFormula(subterms); });
	add("<=", [](const std::vector<fs::Term::cptr>& subterms){ return new fs::LEQAtomicFormula(subterms); });
	add(">",  [](const std::vector<fs::Term::cptr>& subterms){ return new fs::GTAtomicFormula(subterms); });
	add(">=", [](const std::vector<fs::Term::cptr>& subterms){ return new fs::GEQAtomicFormula(subterms); });
	
	// Register the builtin global constraints
	add("@alldiff", [](const std::vector<fs::Term::cptr>& subterms){ return new fs::AlldiffFormula(subterms); });
	add("@sum",     [](const std::vector<fs::Term::cptr>& subterms){ return new fs::SumFormula(subterms); });
	
	add(typeid(fs::AlldiffFormula), [](const fs::AtomicFormula& formula){ return new AlldiffConstraint(formula.getScope()); });
	add(typeid(fs::SumFormula), [](const fs::AtomicFormula& formula){ return new SumConstraint(formula.getScope()); });
}

LogicalComponentRegistry::~LogicalComponentRegistry() {
	 // Delete all the pointers to gecode translators
	for (const auto elem:_gecode_term_translators) delete elem.second;
	for (const auto elem:_gecode_formula_translators) delete elem.second;
}

void LogicalComponentRegistry::add(const std::string& symbol, const FormulaCreator& creator) {
	auto res = _formula_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of formula creator for symbol " + symbol);
}

void LogicalComponentRegistry::add(const std::type_info& type, const DirectFormulaTranslator& creator) {
	auto res = _direct_formula_translators.insert(std::make_pair(std::type_index(type), creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of formula translator for class " + std::string(type.name()));
}

void LogicalComponentRegistry::add(const std::type_info& type, const gecode::TermTranslator::cptr translator) {
	auto res = _gecode_term_translators.insert(std::make_pair(std::type_index(type), translator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of gecode translator for class " + std::string(type.name()));
}

void LogicalComponentRegistry::add(const std::type_info& type, const gecode::AtomicFormulaTranslator::cptr translator) {
	auto res = _gecode_formula_translators.insert(std::make_pair(std::type_index(type), translator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of gecode translator for class " + std::string(type.name()));
}

fs::AtomicFormula::cptr LogicalComponentRegistry::instantiate_formula(const std::string symbol, const std::vector<fs::Term::cptr>& subterms) const {
	auto it = _formula_creators.find(symbol);
	if (it == _formula_creators.end()) throw std::runtime_error("An externally defined symbol '" + symbol + "' is being used without having registered a suitable term/formula creator for it");
	return it->second(subterms);
}

DirectConstraint::cptr LogicalComponentRegistry::instantiate_direct_constraint(const fs::AtomicFormula& formula) const {
	auto it = _direct_formula_translators.find(std::type_index(typeid(formula)));
	if (it == _direct_formula_translators.end()) return nullptr;
	return it->second(formula);
}


gecode::TermTranslator::cptr LogicalComponentRegistry::getGecodeTranslator(const fs::Term& term) const {
	auto it = _gecode_term_translators.find(std::type_index(typeid(term)));
	if (it == _gecode_term_translators.end()) return nullptr;
	return it->second;
}

gecode::AtomicFormulaTranslator::cptr LogicalComponentRegistry::getGecodeTranslator(const fs::AtomicFormula& formula) const {
	auto it = _gecode_formula_translators.find(std::type_index(typeid(formula)));
	if (it == _gecode_formula_translators.end()) return nullptr;
	return it->second;
}

} // namespaces
