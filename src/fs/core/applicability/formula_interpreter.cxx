
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/utils/utils.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 {

FormulaInterpreter* FormulaInterpreter::create(const fs::Formula* formula, const AtomIndex& tuple_index) {
	// If there is some quantified variable in the formula, we will use a CSP-based interpreter
	auto existential_formulae = Utils::filter_by_type<const fs::ExistentiallyQuantifiedFormula*>(fs::all_formulae(*formula));
	if (!existential_formulae.empty()) {
		LPT_INFO("main", "Created a CSP sat. manager for formula: " << *formula);
		throw std::runtime_error("Gecode no longer available");
	} else {
		LPT_INFO("main", "Created a direct sat. manager for formula: " << *formula);
		return new DirectFormulaInterpreter(formula);
	}
}

FormulaInterpreter::FormulaInterpreter(const fs::Formula* formula) :
	_formula(formula->clone())
{}

FormulaInterpreter::~FormulaInterpreter() {
	delete _formula;
}

FormulaInterpreter::FormulaInterpreter(const FormulaInterpreter& other) :
	_formula(other._formula->clone())
{}


bool DirectFormulaInterpreter::satisfied(const State& state) const {
	return _formula->interpret(state);
}


} // namespaces
