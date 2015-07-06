
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <constraints/constraint_manager.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/problem_manager.hxx>

namespace fs0 {

/**
 * 
 */
class GecodeConstraintManager: public BaseConstraintManager
{
public:
	typedef GecodeConstraintManager* cptr;
	
	GecodeConstraintManager(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints);
	~GecodeConstraintManager();
	
	//! Prunes the domains contained in the state by filtering them with the state constraints.
	ScopedConstraint::Output pruneUsingStateConstraints(RelaxedState& state) const;
	
	//! Goal checking for non-relaxed states.
	inline bool isGoal(const State& s) const { return goalConstraintsManager.checkSatisfaction(s); }
	
	inline unsigned numUnsatisfiedGoals( const State& s ) const { return goalConstraintsManager.countUnsatisfied(s); }

	inline unsigned numGoalConstraints() const { return goalConstraintsManager.numConstraints(); }

	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	bool isGoal(const RelaxedState& state) const;
	
protected:
	//! We store here all the constraints that we want to take into account when determining if a relaxed state is a goal:
	//! This includes both the explicit goal constraints plus the state constraints.
	const ScopedConstraint::vcptr allConstraints;
	
	//! A vector with all the variables that are relevant to the goal.
	VariableIdxVector allRelevantVariables;	
	
	const ConstraintManager goalConstraintsManager;
	
	//! True iff there is at least one state constraint.
	bool hasStateConstraints;
	
	//! Variables mapping: For a state variable X, variables[X] is the (implicit, unsigned) ID of corresponding Gecode CSP variable.
	std::map<VariableIdx, unsigned> _variables;
	
	//!
	gecode::SimpleCSP::ptr baseCSP;
	
	//!
	gecode::GecodeCSPTranslator translator;
	
	//! Creates the CSP that corresponds to a given action.
	gecode::SimpleCSP::ptr createCSPVariables();
	
	//! Adds constraints to the csp being managed
	void addDefaultConstraints( const Action& a, const ScopedConstraint::vcptr& stateConstraints  );
	
	//! Helper to extract all relevant variables _uniquely_.
	static VariableIdxSet getAllRelevantVariables(const ScopedConstraint::vcptr& constraints);
	
	//! Returns true iff the goal CSP is solvable. In that case, extracts the goal supports from the first solution
	bool solveCSP(gecode::SimpleCSP* csp, Atom::vctr& support, const State& seed) const;
};

} // namespaces

