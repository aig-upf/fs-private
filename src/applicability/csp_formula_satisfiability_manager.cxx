
#include <applicability/csp_formula_satisfiability_manager.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <state.hxx>
#include <problem.hxx>


namespace fs0 {

CSPFormulaSatisfiabilityManager::CSPFormulaSatisfiabilityManager(const Formula::cptr formula) :
	_csp_handler(new gecode::GecodeFormulaCSPHandler(formula))
{}
	
bool CSPFormulaSatisfiabilityManager::satisfied(const State& state) const {
	gecode::SimpleCSP* csp = _csp_handler->instantiate_csp(state);
	csp->checkConsistency();
	return _csp_handler->check_solution_exists(csp);
}



} // namespaces
