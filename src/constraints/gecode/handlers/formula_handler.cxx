
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/logging.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {
	
GecodeFormulaCSPHandler::GecodeFormulaCSPHandler(const std::vector<AtomicFormula::cptr>& conditions)
	:  GecodeCSPHandler(),
	  _conditions(conditions)
{
	createCSPVariables();
	registerFormulaConstraints(_conditions);
	
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
	_translator.updateStateVariableDomains(*csp, layer);
	return csp;
}


void GecodeFormulaCSPHandler::createCSPVariables() {
	IntVarArgs variables;
	registerFormulaVariables(_conditions,  variables);
	IntVarArray tmp(_base_csp, variables);
	_base_csp._X.update(_base_csp, false, tmp);
}

bool GecodeFormulaCSPHandler::compute_support(SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	DFS<SimpleCSP> engine(csp);
	
	// ATM we are happy to extract the goal support from the first solution
	// TODO An alternative strategy to try out would be to select the solution with most atoms in the seed state, but that implies iterating through all solutions, which might not be worth it?
	SimpleCSP* solution = engine.next();
	if (solution) {
		for (const auto& it:_translator.getAllInputVariables()) {
			VariableIdx planning_variable = it.first;
			const Gecode::IntVar& csp_var = csp->_X[it.second];
			support.push_back(Atom(planning_variable, csp_var.val()));
		}
		delete solution;
		return true;
	}
	return false;
}

void GecodeFormulaCSPHandler::recoverApproximateSupport(gecode::SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	// We have already propagated constraints with the call to status(), so we simply arbitrarily pick one consistent value per variable.
	
	for (const auto& it:_translator.getAllInputVariables()) {
		VariableIdx planning_variable = it.first;
		const Gecode::IntVar& csp_var = csp->_X[it.second];
		IntVarValues values(csp_var);  // This returns a set with all consistent values for the given variable
		assert(values()); // Otherwise the CSP would be inconsistent.
		support.push_back(Atom(planning_variable, values.val()));
	}
}

} } // namespaces
