
#include <cassert>
#include <iosfwd>
#include <complex_action_manager.hxx>
#include <constraints/constraint_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <boost/container/flat_map.hpp>
#include <constraints/gecode/translators.hxx>
#include <constraints/gecode/helper.hxx>
#include <tuple>

#include <gecode/driver.hh>

using namespace Gecode;
using namespace fs0::gecode;

namespace fs0 {

ComplexActionManager::ComplexActionManager(const Action& action, const ScopedConstraint::vcptr& stateConstraints)
	:  BaseActionManager()
{
	baseCSP = createCSPVariables(action, stateConstraints);
	addDefaultConstraints(action, stateConstraints);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = baseCSP->status();
	
	// TODO This should prob. never happened, as it'd mean that the action is (statically) unapplicable.
	assert(st != Gecode::SpaceStatus::SS_FAILED); 
}

ComplexActionManager::~ComplexActionManager() {
	delete baseCSP;
}

void ComplexActionManager::processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg) {
	// MRJ: This is rather ugly, the problem is that clone returns a pointer to Space...
	// it may be a good idea to overwrite this method on SimpleCSP to avoid this downcast.
	// GFM: Can probably be safely changed to a static cast??
	SimpleCSP* csp = dynamic_cast<SimpleCSP::ptr>(baseCSP->clone());

	// Setup domain constraints etc.
	Helper::addRelevantVariableConstraints(*csp, translator, action.getAllRelevantVariables(), layer);
	
	// We do not need to take values that were already achieved in the previous layer into account.
	for ( ScopedEffect::cptr effect : action.getEffects() ) {
		addNoveltyConstraints(effect->getAffected(), layer, *csp);
	}
	
	if (true) {  // Solve the CSP completely
		solveCSP(csp, actionIdx, action, rpg);
	} else { // Check only local consistency
// 		if (!csp->checkConsistency()) return; // We're done
		// TODO - Don't forget to delete the CSP in case of premature exit
		
	}
	
	delete csp;
}



const void ComplexActionManager::solveCSP(gecode::SimpleCSP* csp, unsigned actionIdx, const Action& action, RPGData& rpg) const {
	
	// TODO posting a branching might make sense to prioritize some branching strategy?
    // branch(*this, l, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	DFS<SimpleCSP> engine(csp);
	
	while (SimpleCSP* solution = engine.next()) {
		
		for ( ScopedEffect::cptr effect : action.getEffects() ) {
			VariableIdx affected = effect->getAffected();
			Atom atom(affected, translator.resolveValue(*solution, affected, GecodeCSPTranslator::VariableType::Output)); // TODO - this could be optimized and factored out of the loop
			auto hint = rpg.getInsertionHint(atom);
			
			if (hint.first) { // The value is actually new - let us compute the supports, i.e. the CSP solution values for each variable relevant to the effect.
				Atom::vctrp support = std::make_shared<Atom::vctr>();
				for (VariableIdx scopeVariable: effect->getScope()) {
					support->push_back(Atom(scopeVariable, translator.resolveValue(*solution, scopeVariable, GecodeCSPTranslator::VariableType::Input)));
				}
				rpg.add(atom, actionIdx, support, hint.second);
			}
		}
	}
}


void ComplexActionManager::addNoveltyConstraints(const VariableIdx variable, const RelaxedState& layer, SimpleCSP& csp) {
	// TODO - This could be built incrementally to incorporate values added in this layer by previous actions in the iteration!
	auto& csp_var = translator.resolveVariable(csp, variable, GecodeCSPTranslator::VariableType::Output);
	for (ObjectIdx value:*(layer.getValues(variable))) {
		rel( csp, csp_var, IRT_NQ, value ); // v != value
	}
}


VariableIdxSet ComplexActionManager::getAllRelevantVariables( const Action& a, const ScopedConstraint::vcptr& stateConstraints ) {
	VariableIdxSet variables;
	// Add the variables mentioned by state constraints
	// for ( ScopedConstraint::cptr global : stateConstraints ) variables.insert( global->getScope().begin(), global->getScope().end() );
	// Add the variables mentioned in the preconditions
	for ( ScopedConstraint::cptr precondition : a.getConstraints() ) variables.insert( precondition->getScope().begin(), precondition->getScope().end() );
	// Add the variables appearing in the scope of the effects
	for ( ScopedEffect::cptr effect : a.getEffects() ) variables.insert( effect->getScope().begin(), effect->getScope().end() );
	return variables;
}

VariableIdxSet ComplexActionManager::getAllAffectedVariables(const Action& a) {
	VariableIdxSet variables;
	// Add the variables appearing in the scope of the effects
	for ( ScopedEffect::cptr effect : a.getEffects() ) variables.insert( effect->getAffected() );
	return variables;
}

SimpleCSP::ptr ComplexActionManager::createCSPVariables( const Action& a, const ScopedConstraint::vcptr& stateConstraints ) {
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

	std::cout << "Created SimpleCSP with variables: " << csp->_X << std::endl;
	return csp;
}

void ComplexActionManager::addDefaultConstraints(const Action& a, const ScopedConstraint::vcptr& stateConstraints) {
	// translateConstraints(*baseCSP, translator, stateConstraints); // state constraints
	Helper::translateConstraints(*baseCSP, translator, a.getConstraints()); // Action preconditions
	Helper::translateEffects(*baseCSP, translator, a.getEffects()); // Action preconditions
}

} // namespaces
