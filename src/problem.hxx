
# pragma once

#include <iosfwd>

#include <fs0_types.hxx>
#include <actions.hxx>
#include <state.hxx>
#include <problem_info.hxx>
#include <constraints/constraints.hxx>
#include <simple_applicable_action_set.hxx>
#include <simple_action_set_manager.hxx>
#include <constraints/problem_manager.hxx>

namespace fs0 {

class Problem
{
public:
	Problem() {}
	~Problem() {
		for (ProblemConstraint::cptr ctr:_constraints) delete ctr;
		for (ProblemConstraint::cptr ctr:_gconstraints) delete ctr;
	} 

	//! Modify the problem initial state
	void setInitialState(const State::cptr& state) { _initialState = state; }
	const State::cptr getInitialState() const { return _initialState; }
	
	//! Modify the problem (grounded) actions
	void addAction(const CoreAction::cptr& action) { _actions.push_back(action); }
	const CoreAction::cptr& getAction(ActionIdx idx) const { return _actions.at(idx); }
	unsigned getNumActions() const { return _actions.size(); }
	const ActionList& getAllActions() const { return _actions; }

	SimpleApplicableActionSet getApplicableActions(const State& s) const {
		return SimpleApplicableActionSet(SimpleActionSetManager(s, getConstraints()), _actions);
	}
	
	bool isGoal(const State& s) const { return ctrManager->isGoal(s); }

	
	void registerConstraint(const ProblemConstraint::cptr constraint) { _constraints.push_back(constraint);}
	const ProblemConstraint::vctr& getConstraints() const { return _constraints; }
	void registerGoalConstraint(const ProblemConstraint::cptr constraint) { _gconstraints.push_back(constraint);}
	const ProblemConstraint::vctr& getGoalConstraints() const { return _gconstraints; }

	
	//! Getter/setter for the associated ProblemInfo object.
	void setProblemInfo(const ProblemInfo::cptr& problemInfo) { _problemInfo = problemInfo; }
	const ProblemInfo::cptr getProblemInfo() const { return _problemInfo; }
	
	static void setCurrentProblem(const Problem& problem) {
		_instance = &problem;
	}
	
	static const Problem* getCurrentProblem() {
		assert(_instance);
		return _instance;
	}
	
	void createConstraintManager() {
		ctrManager = std::make_shared<PlanningConstraintManager>(_gconstraints, _constraints);
	}
	
	PlanningConstraintManager::cptr getConstraintManager() const { return ctrManager; }

protected:
	State::cptr _initialState;
	
	PlanningConstraintManager::cptr ctrManager;
	
	ActionList _actions;
	
	ProblemInfo::cptr _problemInfo;
	
	//! Vectors of pointers to the different problem constraints. This class owns the pointers.
	ProblemConstraint::vctr _constraints;
	ProblemConstraint::vctr _gconstraints;
	
	static const Problem* _instance;
};

	  
	  
	  
} // namespaces

