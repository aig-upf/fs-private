
#include <applicability/formula_interpreter.hxx>
#include <languages/fstrips/language.hxx>
#include <utils/utils.hxx>
#include <utils/logging.hxx>
#include <constraints/gecode/handlers/lifted_formula_handler.hxx>

namespace fs0 {

FormulaInterpreter* FormulaInterpreter::create(const fs::Formula* formula, const TupleIndex& tuple_index) {
	// If there is some quantified variable in the formula, we will use a CSP-based interpreter
	auto existential_formulae = Utils::filter_by_type<const fs::ExistentiallyQuantifiedFormula*>(formula->all_formulae());
	if (!existential_formulae.empty()) {
		FINFO("main", "Created a CSP sat. manager for formula: " << *formula);
		// TODO - Note that we are cloning the formula here because otherwise the destructor of the interpreter will attempt to
		// delete it, but the ownership does actually not belong to him.
		return new CSPFormulaInterpreter(formula->clone(), tuple_index);
	} else {
		FINFO("main", "Created a direct sat. manager for formula: " << *formula);
		return new DirectFormulaInterpreter(formula);
	}
}


DirectFormulaInterpreter::DirectFormulaInterpreter(const fs::Formula* formula) :
	_formula(formula)
{}

bool DirectFormulaInterpreter::satisfied(const State& state) const {
	return _formula->interpret(state);
}


CSPFormulaInterpreter::~CSPFormulaInterpreter() {
	delete _formula_handler;
}

CSPFormulaInterpreter::CSPFormulaInterpreter(const fs::Formula* formula, const TupleIndex& tuple_index) :
	// Note that we don't need any of the optimizations, since we will be instantiating the CSP on a state, not a RPG layer
	_formula_handler(new gecode::LiftedFormulaHandler(formula, tuple_index, false))
{}

bool CSPFormulaInterpreter::satisfied(const State& state) const {
	gecode::SimpleCSP* csp = _formula_handler->instantiate(state);
	if (!csp) return false;
	csp->checkConsistency();
	bool sol = _formula_handler->is_satisfiable(csp);
	delete csp;
	return sol;
}

} // namespaces
