
#include <external/repository.hxx>
#include <problem_info.hxx>
#include <utils/logging.hxx>
#include <languages/fstrips/builtin.hxx>
#include <constraints/direct/alldiff_constraint.hxx>
#include <constraints/direct/sum_constraint.hxx>


namespace fs0 {

ExternalComponentRepository& ExternalComponentRepository::instance() {
	static ExternalComponentRepository theInstance;
	return theInstance;
}

ExternalComponentRepository::ExternalComponentRepository() {
	// Register the builtin global constraints
	add("@alldiff", [](const std::vector<fs::Term::cptr>& subterms){ return new fs::AlldiffFormula(subterms); });
	add("@sum",     [](const std::vector<fs::Term::cptr>& subterms){ return new fs::SumFormula(subterms); });
	
	add(typeid(fs::AlldiffFormula), [](const fs::AtomicFormula& formula){ return new AlldiffConstraint(formula.getScope()); });
	add(typeid(fs::SumFormula), [](const fs::AtomicFormula& formula){ return new SumConstraint(formula.getScope()); });
	
	// TODO - Register equivalent Gecode Translators
	
}

void ExternalComponentRepository::add(const std::string& symbol, const FormulaCreator& creator) {
	auto res = _formula_creators.insert(std::make_pair(symbol, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of formula creator for symbol " + symbol);
}

void ExternalComponentRepository::add(TypeInfoRef type, const DirectFormulaTranslator& creator) {
	auto res = _direct_formula_translators.insert(std::make_pair(type, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of formula translator for class " + std::string(type.get().name()));
}

fs::AtomicFormula::cptr ExternalComponentRepository::instantiate_formula(const std::string symbol, const std::vector<fs::Term::cptr>& subterms) const {
	auto it = _formula_creators.find(symbol);
	if (it == _formula_creators.end()) throw std::runtime_error("An externally defined symbol '" + symbol + "' is being used without having registered a suitable term/formula creator for it");
	return it->second(subterms);
}

DirectConstraint::cptr ExternalComponentRepository::instantiate_direct_constraint(const fs::AtomicFormula& formula) const {
	auto it = _direct_formula_translators.find(typeid(formula));
	if (it == _direct_formula_translators.end()) return nullptr;
	return it->second(formula);
}


} // namespaces
