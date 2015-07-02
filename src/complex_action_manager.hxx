
#pragma once

#include <relaxed_action_manager.hxx>
#include <actions.hxx>
#include <fs0_types.hxx>
#include <state.hxx>
#include <constraints/constraint_manager.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <gecode/int.hh>

namespace fs0 {

class RPGData;


/**
 * An action manager based on modeling the action preconditions and effects as a CSP and solving it approximately / completely with Gecode.
 */
class ComplexActionManager : public BaseActionManager
{
protected:
	//!
	gecode::SimpleCSP::ptr baseCSP;
	
	//!
	gecode::GecodeCSPTranslator translator;

public:
	//!
	ComplexActionManager(const Action& action, const ScopedConstraint::vcptr& stateConstraints);
	~ComplexActionManager();

	//!
	void processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg);

protected:

	//!
	const void solveCSP(gecode::SimpleCSP* csp, unsigned actionIdx, const Action& action, RPGData& rpg) const;
		
	//! Creates the SimpleCSP that corresponds to a given action.
	gecode::SimpleCSP::ptr createCSPVariables( const Action& a, const ScopedConstraint::vcptr& stateConstraints );

	//! Adds constraints to the csp being managed
	void addDefaultConstraints( const Action& a, const ScopedConstraint::vcptr& stateConstraints  );
	
	//! Prevents the affected variables to take values already achieved in the previous layer
	void addNoveltyConstraints(const VariableIdx variable, const RelaxedState& layer, gecode::SimpleCSP& csp);
	
	//! A couple of helpers to extract all relevant / affected variables _uniquely_.
	static VariableIdxSet getAllRelevantVariables(const Action& a, const ScopedConstraint::vcptr& stateConstraints);
	static VariableIdxSet getAllAffectedVariables(const Action& a);
};



} // namespaces
