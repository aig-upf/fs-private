
#include <applicability/formula_satisfiability_manager.hxx>
#include <applicability/direct_formula_satisfiability_manager.hxx>
#include <applicability/csp_formula_satisfiability_manager.hxx>

namespace fs0 {

FormulaSatisfiabilityManager* FormulaSatisfiabilityManager::create(const fs::Formula::cptr formula) {
// 	return std::unique_ptr<FormulaSatisfiabilityManager>(new CSPFormulaSatisfiabilityManager(formula));
// 	return std::unique_ptr<FormulaSatisfiabilityManager>(new DirectFormulaSatisfiabilityManager(formula));
	return new DirectFormulaSatisfiabilityManager(formula);
}



} // namespaces
