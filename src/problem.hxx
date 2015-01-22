
# pragma once

#include <iosfwd>

#include <fs0_types.hxx>
#include <state.hxx>
#include <problem_info.hxx>
#include <simple_applicable_action_set.hxx>
#include <simple_action_set_manager.hxx>
#include <constraints/problem_manager.hxx>
#include "constraints/scoped_constraint.hxx"
#include <actions.hxx>
#include <relaxed_applicability_manager.hxx>
#include <relaxed_effect_manager.hxx>

namespace fs0 {

class Problem
{
public:
	Problem();
	~Problem();

	//! Modify the problem initial state
	void setInitialState(const State::cptr& state) { _initialState = state; }
	const State::cptr getInitialState() const { return _initialState; }
	
	//! Modify the problem (grounded) actions
	void addAction(const Action::cptr& action) { _actions.push_back(action); }
	const Action::cptr& getAction(ActionIdx idx) const { return _actions.at(idx); }
	unsigned getNumActions() const { return _actions.size(); }
	const Action::vcptr& getAllActions() const { return _actions; }

	SimpleApplicableActionSet getApplicableActions(const State& s) const {
		return SimpleApplicableActionSet(SimpleActionSetManager(s, getConstraints()), _actions);
	}
	
	bool isGoal(const State& s) const { return ctrManager->isGoal(s); }

	
	void registerConstraint(const ScopedConstraint::cptr constraint) { stateConstraints.push_back(constraint);}
	const ScopedConstraint::vcptr& getConstraints() const { return stateConstraints; }
	void registerGoalConstraint(ScopedConstraint::cptr constraint) { goalConstraints.push_back(constraint);}
	const ScopedConstraint::vcptr& getGoalConstraints() const { return goalConstraints; }

	
	//! Getter/setter for the associated ProblemInfo object.
	void setProblemInfo(const ProblemInfo::cptr& problemInfo) { _problemInfo = problemInfo; }
	const ProblemInfo::cptr getProblemInfo() const { return _problemInfo; }
	
	static void setCurrentProblem(Problem& problem) {
		problem.bootstrap();
		_instance = &problem;
	}
	
	static const Problem* getCurrentProblem() {
		assert(_instance);
		return _instance;
	}
	
	PlanningConstraintManager::cptr getConstraintManager() const { return ctrManager; }
	
	const RelaxedApplicabilityManager& getRelaxedApplicabilityManager() const { return appManager; }
	const RelaxedEffectManager& getRelaxedEffectManager() const { return effManager; }

protected:
	State::cptr _initialState;
	
	PlanningConstraintManager::cptr ctrManager;
	
	Action::vcptr _actions;
	
	ProblemInfo::cptr _problemInfo;
	
	//! Vectors of pointers to the different problem constraints. This class owns the pointers.
	ScopedConstraint::vcptr stateConstraints;
	ScopedConstraint::vcptr goalConstraints;
	
	const RelaxedApplicabilityManager appManager;
	const RelaxedEffectManager effManager;
	
	static const Problem* _instance;

	//! This performs a number of necessary routines once all of the problem information has been defined.
	void bootstrap();
};

	  
	  
	  
} // namespaces

