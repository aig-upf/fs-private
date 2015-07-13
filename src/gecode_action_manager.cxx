
#include <cassert>
#include <iosfwd>
#include <gecode_action_manager.hxx>
#include <constraints/constraint_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <boost/container/flat_map.hpp>
#include <constraints/gecode/translators.hxx>
#include <constraints/gecode/helper.hxx>
#include <utils/logging.hxx>
#include <tuple>

#include <gecode/driver.hh>

using namespace Gecode;
using namespace fs0::gecode;

namespace fs0 {

GecodeActionManager::GecodeActionManager(const Action& action, const ScopedConstraint::vcptr& stateConstraints)
	:  BaseActionManager(), baseCSP(nullptr), translator()
{
	
	baseCSP = createCSPVariables(action, stateConstraints);
	addDefaultConstraints(action, stateConstraints);
	Helper::postBranchingStrategy(*baseCSP);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = baseCSP->status();
	
	// TODO This should prob. never happen, as it'd mean that the action is (statically) unapplicable.
	assert(st != Gecode::SpaceStatus::SS_FAILED); 
}

GecodeActionManager::~GecodeActionManager() {
	delete baseCSP;
}

void GecodeActionManager::processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg) {
	// MRJ: This is rather ugly, the problem is that clone returns a pointer to Space...
	// it may be a good idea to overwrite this method on SimpleCSP to avoid this downcast.
	// GFM: Can probably be safely changed to a static cast??
	SimpleCSP* csp = dynamic_cast<SimpleCSP::ptr>(baseCSP->clone());
	
	FDEBUG("main", "Processing action " << action);

	// Setup domain constraints etc.
	Helper::addRelevantVariableConstraints(*csp, translator, action.getAllRelevantVariables(), layer);
	
	// We do not need to take values that were already achieved in the previous layer into account.
	// TODO - This is not correct yet and needs further thought - e.g. instead of constraints Y not in "set of already achieved values",
	// TODO - the correct thing to do would be a constraint "Y in set of already achieved values OR Y = [w]^k"
// 	for ( ScopedEffect::cptr effect : action.getEffects() ) {
// 		addNoveltyConstraints(effect->getAffected(), layer, *csp);
// 	}
	
	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints
	
	if (!locallyConsistent) {
		FDEBUG("main", "The action CSP is locally inconsistent");
	} else {
		if (true) {  // Solve the CSP completely
			solveCSP(csp, actionIdx, action, rpg);
		} else { // Check only local consistency
			// TODO - Don't forget to delete the CSP in case of premature exit
		}
	}
	delete csp;
}



const void GecodeActionManager::solveCSP(gecode::SimpleCSP* csp, unsigned actionIdx, const Action& action, RPGData& rpg) const {
	unsigned num_solutions = 0;
	DFS<SimpleCSP> engine(csp);

	while (SimpleCSP* solution = engine.next()) {
		for (ScopedEffect::cptr effect : action.getEffects()) {
			VariableIdx affected = effect->getAffected();
			Atom atom(affected, translator.resolveValue(*solution, affected, GecodeCSPTranslator::VariableType::Output)); // TODO - this could be optimized and factored out of the loop
			auto hint = rpg.getInsertionHint(atom);
			
			if (hint.first) { // The value is actually new - let us compute the supports, i.e. the CSP solution values for each variable relevant to the effect.
				Atom::vctrp atomSupport = std::make_shared<Atom::vctr>();
				Atom::vctrp actionSupport = std::make_shared<Atom::vctr>();
				boost::container::flat_set<VariableIdx> processed;
				
				// TODO - The set of variables related to the support of each effect could indeed be precomputed in order to speed up this.
				// First extract the supports related to the variables relevant to the particular effect
				for (VariableIdx variable: effect->getScope()) {
					atomSupport->push_back(Atom(variable, translator.resolveValue(*solution, variable, GecodeCSPTranslator::VariableType::Input)));
					processed.insert(variable);
				}
				
				// And now those related to the variables relevant to the action precondition - without repeating those that might already have been part of the effect.
				for (VariableIdx variable: action.getScope()) {
					if (processed.find(variable) == processed.end()) {
						actionSupport->push_back(Atom(variable, translator.resolveValue(*solution, variable, GecodeCSPTranslator::VariableType::Input)));
					}
				}
				
				// Once the support is computed, we insert the new atom into the RPG data structure
				rpg.add(atom, actionIdx, actionSupport, atomSupport, hint.second);
			}
		}
		++num_solutions;
		delete solution;
	}
	
	FDEBUG("main", "Solving the Action CSP completely produced " << num_solutions << " solutions"  << std::endl << *this);
}


void GecodeActionManager::addNoveltyConstraints(const VariableIdx variable, const RelaxedState& layer, SimpleCSP& csp) {
	// TODO - This could be built incrementally to incorporate values added in this layer by previous actions in the iteration!
	auto& csp_var = translator.resolveVariable(csp, variable, GecodeCSPTranslator::VariableType::Output);
	for (ObjectIdx value:*(layer.getValues(variable))) {
		rel( csp, csp_var, IRT_NQ, value ); // v != value
	}
}


VariableIdxSet GecodeActionManager::getAllRelevantVariables( const Action& a, const ScopedConstraint::vcptr& stateConstraints ) {
	VariableIdxSet variables;
	// Add the variables mentioned by state constraints
	// for ( ScopedConstraint::cptr global : stateConstraints ) variables.insert( global->getScope().begin(), global->getScope().end() );
	// Add the variables mentioned in the preconditions
	for ( ScopedConstraint::cptr precondition : a.getConstraints() ) variables.insert( precondition->getScope().begin(), precondition->getScope().end() );
	// Add the variables appearing in the scope of the effects
	for ( ScopedEffect::cptr effect : a.getEffects() ) variables.insert( effect->getScope().begin(), effect->getScope().end() );
	return variables;
}

VariableIdxSet GecodeActionManager::getAllAffectedVariables(const Action& a) {
	VariableIdxSet variables;
	// Add the variables appearing in the scope of the effects
	for ( ScopedEffect::cptr effect : a.getEffects() ) variables.insert( effect->getAffected() );
	return variables;
}

SimpleCSP::ptr GecodeActionManager::createCSPVariables( const Action& a, const ScopedConstraint::vcptr& stateConstraints ) {
	// Determine input and output variables for this action: we first amalgamate variables into a set
	// to avoid repetitions, then generate corresponding CSP variables, then create the CSP model with them
	// and finally add the model constraints.

	SimpleCSP::ptr csp = new SimpleCSP;

	IntVarArgs variables;
	// Index the relevant variables first
	for (VariableIdx var:getAllRelevantVariables(a, stateConstraints)) {
		unsigned id = Helper::processVariable( *csp, var, variables );
		translator.registerCSPVariable(var, GecodeCSPTranslator::VariableType::Input, id);
	}

	// Index the affected variables next
	for (VariableIdx var:getAllAffectedVariables(a)) {
		unsigned id = Helper::processVariable( *csp, var, variables );
		translator.registerCSPVariable(var, GecodeCSPTranslator::VariableType::Output, id);
	}

	IntVarArray tmp( *csp, variables );
	csp->_X.update( *csp, false, tmp );

	return csp;
}

void GecodeActionManager::addDefaultConstraints(const Action& a, const ScopedConstraint::vcptr& stateConstraints) {
	// translateConstraints(*baseCSP, translator, stateConstraints); // state constraints
	Helper::translateConstraints(*baseCSP, translator, a.getConstraints()); // Action preconditions
	Helper::translateEffects(*baseCSP, translator, a.getEffects()); // Action preconditions
}

std::ostream& GecodeActionManager::print(std::ostream& os) const {
	return translator.print(os, *baseCSP);
}


} // namespaces
