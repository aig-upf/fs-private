
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/utils/utils.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>

namespace fs0 {

FormulaInterpreter* FormulaInterpreter::create(const fs::Formula* formula, const AtomIndex& tuple_index) {
	// If there is some quantified variable in the formula, we will use a CSP-based interpreter
	auto existential_formulae = Utils::filter_by_type<const fs::ExistentiallyQuantifiedFormula*>(fs::all_formulae(*formula));
	if (!existential_formulae.empty()) {
		LPT_INFO("main", "Created a CSP sat. manager for formula: " << *formula);
		// TODO - Note that we are cloning the formula here because otherwise the destructor of the interpreter will attempt to
		// delete it, but the ownership does actually not belong to him.
		return new CSPFormulaInterpreter(formula->clone(), tuple_index);
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

CSPFormulaInterpreter::CSPFormulaInterpreter(const fs::Formula* formula, const AtomIndex& tuple_index) :
	FormulaInterpreter(formula),
	// Note that we don't need any of the optimizations, since we will be instantiating the CSP on a state, not a RPG layer
	_formula_csp(new gecode::FormulaCSP(formula, tuple_index, false)),
	_tuple_index(tuple_index)
{}

CSPFormulaInterpreter::~CSPFormulaInterpreter() {
	delete _formula_csp;
}

CSPFormulaInterpreter::CSPFormulaInterpreter(const CSPFormulaInterpreter& other) :
	FormulaInterpreter(other),
	_formula_csp(new gecode::FormulaCSP(other._formula, other._tuple_index, false)),
	_tuple_index(other._tuple_index)
{}

bool CSPFormulaInterpreter::satisfied(const State& state) const {
	gecode::GecodeCSP* csp = _formula_csp->instantiate(state);
	if (!csp) return false;
	csp->checkConsistency();
	bool sol = _formula_csp->is_satisfiable(csp);
	delete csp;
	return sol;
}

} // namespaces
