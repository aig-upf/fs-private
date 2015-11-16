
#include <applicability/csp_formula_interpreter.hxx>
#include <constraints/gecode/handlers/formula_handler.hxx>
#include <languages/fstrips/language.hxx>


namespace fs0 {

CSPFormulaInterpreter::CSPFormulaInterpreter(const fs::Formula::cptr formula) :
	_csp_handler(new gecode::FormulaCSPHandler(formula, true, false))
{}
	
bool CSPFormulaInterpreter::satisfied(const State& state) const {
	gecode::SimpleCSP* csp = _csp_handler->instantiate_csp(state);
	csp->checkConsistency();
	return _csp_handler->check_solution_exists(csp);
}



} // namespaces
