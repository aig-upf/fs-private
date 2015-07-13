

#include <constraints/gecode_constraint_manager.hxx>
#include <constraints/gecode/helper.hxx>
#include <problem.hxx>
#include <utils/projections.hxx>
#include <utils/utils.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>

#include <gecode/driver.hh>

using namespace Gecode;
using namespace fs0::gecode;

namespace fs0 {

// Note that we use both types of constraints as goal constraints
GecodeConstraintManager::GecodeConstraintManager(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints) :
	allConstraints(Utils::merge(goalConstraints, stateConstraints)),
	allRelevantVariables(),
	goalConstraintsManager(allConstraints), // We store all the constraints in a new vector so that we can pass a const reference 
	                                            // - we're strongly interested in the ConstraintManager having only a reference, not the actual value,
	                                            // since in some cases each grounded action will have a ConstraintManager
	hasStateConstraints(stateConstraints.size() > 0),
	baseCSP(nullptr), translator()
{
	auto relevantSet = getAllRelevantVariables(allConstraints);
	allRelevantVariables = VariableIdxVector(relevantSet.begin(), relevantSet.end());
	
	baseCSP = createCSPVariables();
	
	Helper::translateConstraints(*baseCSP, translator, allConstraints);
	
	Helper::postBranchingStrategy(*baseCSP);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = baseCSP->status();
	
	if (st == Gecode::SpaceStatus::SS_SOLVED) {
		FINFO("main", "The Goal CSP was statically solved:" << std::endl <<  *this);
	} else if (st == Gecode::SpaceStatus::SS_FAILED) {
		// TODO This should prob. never happened, as it'd mean that the action is (statically) unapplicable.
		throw std::runtime_error("Goal CSP statically failed");
	} else {
		FINFO("main", "Goal CSP after the initial, static propagation: " << *this);
	}
	
}

GecodeConstraintManager::~GecodeConstraintManager() {
	delete baseCSP;
}

ScopedConstraint::Output GecodeConstraintManager::pruneUsingStateConstraints(RelaxedState& state) const {
	if (!hasStateConstraints) return ScopedConstraint::Output::Unpruned;
	
	// TODO TODO TODO
	
	return ScopedConstraint::Output::Unpruned;
}




bool GecodeConstraintManager::isGoal(const State& seed, const RelaxedState& layer, Atom::vctr& support) const {
	assert(support.empty());
	
	SimpleCSP* csp = dynamic_cast<SimpleCSP::ptr>(baseCSP->clone());

	// Setup domain constraints etc.
	Helper::addRelevantVariableConstraints(*csp, translator, allRelevantVariables, layer);

	if (!csp->checkConsistency()) {
		delete csp;
		return false;
	}
	
	bool res;
	if (Config::instance().getGoalResolutionType() == Config::CSPResolutionType::Full) {  // Solve the CSP completely
		res = solveCSP(csp, support, seed);
	} else { // Check only local consistency
		res = true;
		recoverApproximateSupport(csp, support, seed);
	}
	
	delete csp;
	return res;
}


//! Don't care about supports, etc.
bool GecodeConstraintManager::isGoal(const RelaxedState& layer) const {
	Atom::vctr dummy;
	State dummy_state(0 ,dummy);
	return isGoal(dummy_state, layer, dummy);
}


SimpleCSP::ptr GecodeConstraintManager::createCSPVariables() {
	// Determine input and output variables for this action: we first amalgamate variables into a set
	// to avoid repetitions, then generate corresponding CSP variables, then create the CSP model with them
	// and finally add the model constraints.

	SimpleCSP::ptr csp = new SimpleCSP;

	// Index all the variables relevant to the goal
	IntVarArgs variables;
	for (VariableIdx var:allRelevantVariables) {
		unsigned id = Helper::processVariable( *csp, var, variables );
		translator.registerCSPVariable(var, GecodeCSPTranslator::VariableType::Input, id);
	}

	IntVarArray tmp(*csp, variables);
	csp->_X.update(*csp, false, tmp);

	return csp;
}

VariableIdxSet GecodeConstraintManager::getAllRelevantVariables(const ScopedConstraint::vcptr& constraints) {
	VariableIdxSet variables;
	for (ScopedConstraint::cptr constraint : constraints) variables.insert(constraint->getScope().begin(), constraint->getScope().end());
	return variables;
}

bool GecodeConstraintManager::solveCSP(gecode::SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	DFS<SimpleCSP> engine(csp);
	
	// ATM we are happy to extract the goal support from the first solution
	// TODO An alternative strategy to try out would be to select the solution with most atoms in the seed state, but that implies iterating through all solutions, which might not be worth it?
	SimpleCSP* solution = engine.next();
	if (solution) {
		for (VariableIdx variable:allRelevantVariables) {
			support.push_back(Atom(variable, translator.resolveValue(*solution, variable, GecodeCSPTranslator::VariableType::Input)));
		}
		delete solution;
		return true;
	}
	return false;
}

void GecodeConstraintManager::recoverApproximateSupport(gecode::SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	//TODO
}


} // namespaces

