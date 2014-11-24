
#ifndef __APTK_CORE_PROBLEM__
#define __APTK_CORE_PROBLEM__

#include <iosfwd>

#include <core_types.hxx>
#include <actions.hxx>
#include <state.hxx>
#include <problem_info.hxx>
#include <applicable_action_set.hxx>
#include <constraints/constraints.hxx>

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
	const CoreAction::cptr& getAction(BoundActionIdx idx) const { return _actions.at(idx); }
	unsigned getNumActions() const { return _actions.size(); }	
	

	ApplicableActionSet<SimpleActionSetManager> computeBoundApplicableActions(const State& s) const {
		return ApplicableActionSet<SimpleActionSetManager>(SimpleActionSetManager(s), _actions);
	}
	
	ApplicableActionSet<RelaxedActionSetManager> computeBoundApplicableActions(const State* s0, const RelaxedState& s) const { 
		return ApplicableActionSet<RelaxedActionSetManager>(RelaxedActionSetManager(s0, s), _actions);
	}
	
	bool isGoal(const State& s) const { 
		SimpleActionSetManager manager(s);
		return manager.isApplicable(*getGoalEvaluator());
	}
	
	bool isGoal(const RelaxedState& s) const { 
		RelaxedActionSetManager manager(s);
		JustifiedApplicableEntity justified(*getGoalEvaluator());
		return manager.isApplicable(justified);
	}
	
	void registerConstraint(const Constraint::cptr constraint) { _constraints.push_back(constraint);}
	const std::vector<Constraint::cptr>& getConstraints() const { return _constraints; }
	
	//! Prints a representation of the predicate to the given stream.
	std::ostream& print(std::ostream& os) const {
		os << "Problem[NOT IMPLEMENTED YET]";
		return os;
	}
	
	friend std::ostream& operator<<(std::ostream &os, const Problem&  problem) {  return problem.print(os); }
	
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
	
	std::vector<Constraint::cptr> _constraints;
	
	static const Problem* _instance;
};

	  
	  
	  
} } // namespaces

#endif
