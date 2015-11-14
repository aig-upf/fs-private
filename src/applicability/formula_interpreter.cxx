
#include <applicability/formula_interpreter.hxx>
#include <applicability/direct_formula_interpreter.hxx>
#include <applicability/csp_formula_interpreter.hxx>
#include <languages/fstrips/language.hxx>
#include <utils/utils.hxx>
#include <utils/logging.hxx>

namespace fs0 {

FormulaInterpreter* FormulaInterpreter::create(const fs::Formula::cptr formula) {
	// If there is some quantified variable in the formula, we will use a CSP-based interpreter
	auto existential_formulae = Utils::filter_by_type<fs::ExistentiallyQuantifiedFormula::cptr>(formula->all_formulae());
	if (!existential_formulae.empty()) {
		FINFO("main", "Created a CSP sat. manager for formula: " << *formula);
		// TODO - Note that we are cloning the formula here because otherwise the destructor of the interpreter will attempt to
		// delete it, but the ownership does actually not belong to him.
		return new CSPFormulaInterpreter(formula->clone());
	} else {
		FINFO("main", "Created a direct sat. manager for formula: " << *formula);
		return new DirectFormulaInterpreter(formula);
	}
}




} // namespaces
