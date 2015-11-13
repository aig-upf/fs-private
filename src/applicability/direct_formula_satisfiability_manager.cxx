
#include <applicability/direct_formula_satisfiability_manager.hxx>
#include <state.hxx>

namespace fs0 {

DirectFormulaSatisfiabilityManager::DirectFormulaSatisfiabilityManager(const fs::Formula::cptr formula) :
	_formula(formula)
{}

bool DirectFormulaSatisfiabilityManager::satisfied(const State& state) const {
	return _formula->interpret(state);
}



} // namespaces
