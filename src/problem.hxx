
#pragma once

#include <fs0_types.hxx>
#include <actions/applicable_action_set.hxx>
#include <actions/action_schema.hxx>
#include <actions/ground_action.hxx>

namespace fs0 {

class State; class FormulaSatisfiabilityManager;

class Problem {
public:

	Problem(State* init, const std::vector<ActionSchema::cptr>& schemata, const Formula::cptr goal, const Formula::cptr state_constraints);
	~Problem();

	//! Get the initial state of the problem
	const State& getInitialState() const { return *_init; }

	//! Get the set of action schemata of the problem
	const std::vector<ActionSchema::cptr>& getActionSchemata() const { return _schemata; }
	
	//! Get the set of ground actions of the problem
	const std::vector<GroundAction::cptr>& getGroundActions() const { return _ground; }
	void setGroundActions(std::vector<GroundAction::cptr>&& ground) { _ground = std::move(ground); }
	
	//! Get an iterator on the set of actions that are applicable in the given state
	ApplicableActionSet getApplicableActions(const State& s) const;
	
	//! Get the problem's goal formula
	const Formula::cptr getGoalConditions() const { return _goal_formula; }
	
	//! Get the state constraint formula of the problem
	const Formula::cptr getStateConstraints() const { return _state_constraint_formula; }
	
	//! Get the name of the action with given index
	std::string get_action_name(unsigned action) const;

	//! Set the global singleton problem instance
	static void setInstance(std::unique_ptr<Problem>&& problem) {
		assert(!_instance);
		_instance = std::move(problem);
	}
	
	//! Global singleton object accessor
	static const Problem& getInstance() {
		assert(_instance);
		return *_instance;
	}
	
	//! Get the ProblemInfo object associated to this problem.
	const ProblemInfo& getProblemInfo() const { return getInfo(); }
	
	//! Helper to access the problem info more easily
	
	static void setInfo(ProblemInfo* info) {
		assert(!_info);
		_info = std::unique_ptr<ProblemInfo>(info);
	}
	
	static const ProblemInfo& getInfo() {
		assert(_info);
		return *_info;
	}

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Problem& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	//! The initial state of the problem
	const std::unique_ptr<State> _init;

	// The set of action schemata
	const std::vector<ActionSchema::cptr> _schemata;
	
	// The set of grounded actions of the problem
	std::vector<GroundAction::cptr> _ground;
	
	
	
	//! Pointers to the goal and state constraints formulas. This class owns the pointers.
	const Formula::cptr _state_constraint_formula;
	const Formula::cptr _goal_formula;

	std::unique_ptr<FormulaSatisfiabilityManager> _goal_sat_manager;
	
	//! The singleton instance
	static std::unique_ptr<Problem> _instance;
	
	//! An object with all sorts of extra book-keeping information - currently a singleton instance as well :-(
	static std::unique_ptr<ProblemInfo> _info;
};

} // namespaces
