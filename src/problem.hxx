
# pragma once

#include <iosfwd>
#include <boost/concept_check.hpp>

#include <fs0_types.hxx>
#include <state.hxx>
#include <problem_info.hxx>
#include <simple_applicable_action_set.hxx>
#include <standard_applicability_manager.hxx>
#include <constraints/problem_manager.hxx>
#include "constraints/scoped_constraint.hxx"
#include <actions.hxx>
#include <constraints/compiled.hxx>

namespace fs0 {

class Problem
{
public:
	//! Constructs a problem by loading the problem data from the given directory.
	Problem(const rapidjson::Document& data);
	~Problem();

	//! Modify the problem initial state
	void setInitialState(const State::cptr& state) { _initialState = state; }
	const State::cptr getInitialState() const { return _initialState; }

	//! Modify the problem (grounded) actions
	void addAction(const Action::cptr& action) { _actions.push_back(action); }
	const Action::cptr& getAction(ActionIdx idx) const { return _actions.at(idx); }
	unsigned getNumActions() const { return _actions.size(); }
	const Action::vcptr& getAllActions() const { return _actions; }

	SimpleApplicableActionSet getApplicableActions(const State& s) const;

	bool isGoal(const State& s) const { return ctrManager->isGoal(s); }

	unsigned numUnsatisfiedGoals( const State& s ) const { return ctrManager->numUnsatisfiedGoals( s ); }

	unsigned numGoalConstraints() const { return ctrManager->numGoalConstraints(); }

	void registerConstraint(const ScopedConstraint::cptr constraint) { stateConstraints.push_back(constraint);}
	const ScopedConstraint::vcptr& getConstraints() const { return stateConstraints; }
	void registerGoalConstraint(ScopedConstraint::cptr constraint) { goalConstraints.push_back(constraint);}
	const ScopedConstraint::vcptr& getGoalConstraints() const { return goalConstraints; }

	const std::string& get_action_name(unsigned action) const { return _problemInfo.getActionName(action); }

	//! Simple relevance analysis that over-approximates the set of state variables whose
	//! values can matter for the goal being achievable
	void analyzeVariablesRelevance();

	//! Getter/setter for the associated ProblemInfo object.
	const ProblemInfo& getProblemInfo() const { return _problemInfo; }

	static void setCurrentProblem(Problem& problem) {
		_instance = &problem;
		problem.bootstrap();
	}

	static const Problem* getCurrentProblem() {
		if (!_instance) throw std::runtime_error("Problem has not been instantiated yet");
		return _instance;
	}

	BaseConstraintManager::cptr getConstraintManager() const { return ctrManager; }

	void addDomainBoundConstraints();

	void compileConstraints();

	//! Tells if the given variable is relevant for the goal.
	static bool isRelevantForGoal(VariableIdx var) { return getCurrentProblem()->_goalRelevantVars[var]; }

protected:
	State::cptr _initialState;

	BaseConstraintManager::cptr ctrManager;

	Action::vcptr _actions;

	const ProblemInfo _problemInfo;

	//! Vectors of pointers to the different problem constraints. This class owns the pointers.
	ScopedConstraint::vcptr stateConstraints;
	ScopedConstraint::vcptr goalConstraints;

	static const Problem* _instance;

	//! This performs a number of necessary routines once all of the problem information has been defined.
	void bootstrap();

	//! A helper that compiles in-place a vector of constraints. Returns how many constraints were actually compiled.
	unsigned compileConstraintVector(ScopedConstraint::vcptr& constraints) const;


	//! Bitmap holding what variables are relevant to the goal (i.e. appear on Goal, Precondition or Global constraints)
	std::vector<bool>		_goalRelevantVars;
};




} // namespaces
