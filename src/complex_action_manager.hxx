
#pragma once

#include <relaxed_action_manager.hxx>
#include <actions.hxx>
#include <fs0_types.hxx>
#include <state.hxx>
#include <constraints/constraint_manager.hxx>
#include <constraints/gecode/action_csp.hxx>
#include <gecode/int.hh>

namespace fs0 {

class Problem; class RPGData;


/**
 * An action manager based on modeling the action preconditions and effects as a CSP and solving it approximately / completely with Gecode.
 */
class ComplexActionManager : public BaseActionManager
{
protected:
	gecode::ActionCSP::ptr baseCSP;

	typedef std::map<VariableIdx, unsigned> VariableMap;
	//! Input variables mapping: For a state variable X, inputVariables[X] is the (implicit, unsigned) ID of corresponding Gecode CSP variable.
	VariableMap inputVariables;

	//! Output variables mapping
	VariableMap outputVariables;

	const Gecode::IntVar& resolveVariableName( VariableIdx varName, const Gecode::IntVarArray& actualVars, const VariableMap& map ) const;

public:
	enum class VariableType {Input, Output};
	
	//!
	ComplexActionManager(const Problem& problem, const Action& action);
	~ComplexActionManager();

	//!
	void processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg);

	gecode::ActionCSP::ptr getCSP() { return baseCSP; }

	//! Returns the handle to the Gecode variable corresponding to the
	//! planning state variable varName featured on the r.h.s. of an effect
	const Gecode::IntVar&  resolveX( VariableIdx varName ) const;

	//! Returns the handle to the Gecode variable corresponding to the affected planning state variable varName
	const Gecode::IntVar&  resolveY( VariableIdx varName ) const;

	void addEqualityConstraint(gecode::ActionCSP& csp,  VariableIdx varName, bool value);

	// MRJ: This overload will be necessary as soon as int and ObjectIdx cease to be the same thing
	//void addEqualityConstraint(gecode::ActionCSP& csp,  VariableIdx varName, int  value );

	//! Adds constraint of the form $varName = value$ to the CSP
	void addEqualityConstraint(gecode::ActionCSP& csp,  VariableIdx varName, ObjectIdx value);

	//! Adds constraint of the form $varName \in values$ to the CSP
	void addMembershipConstraint(gecode::ActionCSP& csp,  VariableIdx varName, DomainPtr values);

	//! Adds constraint of the form $lb <= varName <= ub$ to the CSP
	void addBoundsConstraint(gecode::ActionCSP& csp,  VariableIdx varName, int lb, int ub);

	//! Adds constraint of the form $min <= varName <= max$ to the CSP,
	//! where min and max are the minimum and maximum values defined for
	//! the type of varName.
	void addBoundsConstraintFromDomain(gecode::ActionCSP& csp,  VariableIdx varName);

	//!
	const ObjectIdx resolveValue(gecode::ActionCSP& csp, VariableIdx variable, ComplexActionManager::VariableType type) const;
	
protected:

	//!
	static unsigned processVariable(gecode::ActionCSP& csp, VariableIdx var, Gecode::IntVarArgs& varArray);

	//!
	const void solveCSP(gecode::ActionCSP* csp, unsigned actionIdx, const Action& action, RPGData& rpg) const;
		
	//! Creates the ActionCSP that corresponds to a given action.
	gecode::ActionCSP::ptr createCSPVariables( const Action& a, const ScopedConstraint::vcptr& globalConstraints );

	//! Adds constraints to the csp being managed
	void addDefaultConstraints( const Action& a, const ScopedConstraint::vcptr& globalConstraints  );
	
	//! Adds the RPG-layer-dependent constraints to the CSP.
	void addRelevantVariableConstraints(const VariableIdxVector& scope, const RelaxedState& layer, gecode::ActionCSP& csp);
	
	//! Prevents the affected variables to take values already achieved in the previous layer
	void addNoveltyConstraints(const VariableIdx variable, const RelaxedState& layer, gecode::ActionCSP& csp);
};



} // namespaces
