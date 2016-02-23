
#include <applicability/csp_formula_interpreter.hxx>
#include <constraints/gecode/handlers/formula_handler.hxx>
#include <languages/fstrips/language.hxx>


namespace fs0 {

CSPFormulaInterpreter::CSPFormulaInterpreter(const fs::Formula::cptr formula) :
	// Note that we don't need any of the optimizations, since we will be instantiating the CSP on a state, not a RPG layer
	_csp_handler(new gecode::FormulaCSPHandler(formula, false, false, false))
{}
	
bool CSPFormulaInterpreter::satisfied(const State& state) const {
	gecode::SimpleCSP* csp = _csp_handler->instantiate_csp(state);
	if (!csp) return false;
	csp->checkConsistency();
	return _csp_handler->check_solution_exists(csp);
}



} // namespaces
