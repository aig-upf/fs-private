
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/logging.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {
	
GecodeFormulaCSPHandler::GecodeFormulaCSPHandler(const std::vector<AtomicFormula::cptr>& conditions)
	:  GecodeCSPHandler(),
	  relevantVariables(extractRelevantVariables(conditions)),
	  formulaTranslator(conditions, relevantVariables, _base_csp, translator)
{
	createCSPVariables();
	registerConstraints();
	Helper::postBranchingStrategy(_base_csp);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = _base_csp.status();
	
	if (st == Gecode::SpaceStatus::SS_SOLVED) {
		FINFO("main", "The Goal CSP was statically solved:" << std::endl <<  *this);
	} else if (st == Gecode::SpaceStatus::SS_FAILED) { // This should never happen, as it'd mean that the action is (statically) unapplicable.
		throw std::runtime_error("Goal CSP statically failed");
	} else {
		FINFO("main", "Goal CSP after the initial, static propagation: " << *this);
	}
}


SimpleCSP::ptr GecodeFormulaCSPHandler::instantiate_csp(const RelaxedState& layer) const {
	SimpleCSP* csp = dynamic_cast<SimpleCSP::ptr>(_base_csp.clone());
	Helper::addRelevantVariableConstraints(*csp, translator, relevantVariables, layer);
	return csp;
}


void GecodeFormulaCSPHandler::createCSPVariables() {
	IntVarArgs variables;
	formulaTranslator.registerVariables(variables);
	IntVarArray tmp(_base_csp, variables);
	_base_csp._X.update(_base_csp, false, tmp);
}

void GecodeFormulaCSPHandler::registerConstraints() {
// 	Helper::translateConstraints(_base_csp, translator, conditions);
	formulaTranslator.registerConstraints();
}

VariableIdxVector GecodeFormulaCSPHandler::extractRelevantVariables(const std::vector<AtomicFormula::cptr>& conditions) {
	std::set<VariableIdx> relevant;
	for (const AtomicFormula::cptr formula:conditions) {
		VariableIdxVector scope = formula->getScope();
		relevant.insert(scope.begin(), scope.end());
	}
	return VariableIdxVector(relevant.begin(), relevant.end());
}



bool GecodeFormulaCSPHandler::compute_support(SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	DFS<SimpleCSP> engine(csp);
	
	// ATM we are happy to extract the goal support from the first solution
	// TODO An alternative strategy to try out would be to select the solution with most atoms in the seed state, but that implies iterating through all solutions, which might not be worth it?
	SimpleCSP* solution = engine.next();
	if (solution) {
		for (VariableIdx variable:relevantVariables) {
			support.push_back(Atom(variable, translator.resolveValue(*solution, variable, CSPVariableType::Input)));
		}
		delete solution;
		return true;
	}
	return false;
}

void GecodeFormulaCSPHandler::recoverApproximateSupport(gecode::SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	// We have already propagated constraints with the call to status(), so we simply arbitrarily pick one consistent value per variable.
	for (VariableIdx variable:relevantVariables) {
		IntVarValues values(translator.resolveVariable(*csp, variable, CSPVariableType::Input)); 
		assert(values()); // Otherwise the CSP would be inconsistent.
		support.push_back(Atom(variable, values.val()));
	}
}

} } // namespaces
