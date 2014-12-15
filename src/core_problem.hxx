
# pragma once

#include <iosfwd>

#include <core_types.hxx>
#include <actions.hxx>
#include <state.hxx>
#include <problem_info.hxx>
#include <constraints/constraints.hxx>
#include <simple_applicable_action_set.hxx>
#include <simple_action_set_manager.hxx>
#include <relaxed_action_set_manager.hxx>

namespace aptk { namespace core {

class Problem
{
public:
	Problem() {}
	~Problem() {} 

	//! Modify the problem initial state
	void setInitialState(const State::cptr& state) { _initialState = state; }
	const State::cptr getInitialState() const { return _initialState; }
	
	//! Modify the problem goal evaluation routine
	void setGoalEvaluator(const ApplicableEntity::cptr& goal_evaluator) { _goal_evaluator = goal_evaluator; }
	ApplicableEntity::cptr getGoalEvaluator() const { return _goal_evaluator; }
	
	//! Modify the problem (grounded) actions
	void addAction(const CoreAction::cptr& action) { _actions.push_back(action); }
	const CoreAction::cptr& getAction(ActionIdx idx) const { return _actions.at(idx); }
	unsigned getNumActions() const { return _actions.size(); }
	const ActionList& getAllActions() const { return _actions; }

	SimpleApplicableActionSet getApplicableActions(const State& s) const {
		return SimpleApplicableActionSet(SimpleActionSetManager(s, getConstraints()), _actions);
	}
	
	bool isGoal(const State& s) const { 
		SimpleActionSetManager manager(s, getConstraints());
		return manager.isApplicable(*getGoalEvaluator());
	}
	
	bool isGoal(const RelaxedState& s) const { // TODO TODO TODO REFACTOR OUT OF HERE
		RelaxedActionSetManager manager(getConstraints());
		// We compute the projection of the current relaxed state to the variables relevant to the action
		DomainSet projection = manager.projectValues(s, *getGoalEvaluator());
		auto res = manager.isApplicable(*getGoalEvaluator(), projection);
		return res.first;
	}
	
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

protected:
	State::cptr _initialState;
	
	ApplicableEntity::cptr _goal_evaluator;
	
	ActionList _actions;
	
	ProblemInfo::cptr _problemInfo;
	
	ProblemConstraint::vctr _constraints;
	ProblemConstraint::vctr _gconstraints;
	
	static const Problem* _instance;
};

	  
	  
	  
} } // namespaces

