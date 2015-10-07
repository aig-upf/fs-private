
#pragma once

#include <iosfwd>

#include <fs0_types.hxx>
#include <state.hxx>

#include <actions/applicable_action_set.hxx>
#include <actions/action_schema.hxx>
#include <actions/ground_action.hxx>

namespace fs0 {

class Problem
{
public:
	//! Constructs a problem by loading the problem data from the given directory.
	Problem();
	~Problem();

	//! Modify the problem initial state
	void setInitialState(const State::cptr& state) { _initialState = state; }
	const State::cptr getInitialState() const { return _initialState; }

	void addActionSchema(const ActionSchema::cptr action) { _schemata.push_back(action); }
	const std::vector<ActionSchema::cptr>& getActionSchemata() const { return _schemata; }
	
	const std::vector<GroundAction::cptr>& getGroundActions() const { return _ground; }
	void setGroundActions(std::vector<GroundAction::cptr>&& ground) { _ground = std::move(ground); }
	
	ApplicableActionSet getApplicableActions(const State& s) const;
	
	void setStateConstraints(const std::vector<AtomicFormula::cptr>& constraints) { _stateConstraints = constraints;}
	const std::vector<AtomicFormula::cptr>& getStateConstraints() const { return _stateConstraints; }
	void setGoalConditions(const std::vector<AtomicFormula::cptr>& conditions) { _goalConditions = conditions;}
	const std::vector<AtomicFormula::cptr>& getGoalConditions() const { return _goalConditions; }

	std::string get_action_name(unsigned action) const;

	//! Getter/setter for the associated ProblemInfo object.
	void setProblemInfo(ProblemInfo* info) { _problemInfo = info; }
	const ProblemInfo& getProblemInfo() const {
		assert(_problemInfo);
		return *_problemInfo;
	}

	static void setInstance(std::unique_ptr<Problem>&& problem) {
		assert(!_instance);
		_instance = std::move(problem);
	}
	
	//! const version of the singleton accessor
	static const Problem& getInstance() {
		assert(_instance);
		return *_instance;
	}
	
	//! Helper to access the problem info more easily
	static const ProblemInfo& getInfo() { return getInstance().getProblemInfo(); }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Problem& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	//! The initial state of the problem
	State::cptr _initialState;

	// The set of action schemata
	std::vector<ActionSchema::cptr> _schemata;
	
	// The set of grounded actions of the problem
	std::vector<GroundAction::cptr> _ground;
	
	//! An object with all sorts of extra book-keeping information
	ProblemInfo* _problemInfo;
	
	//! Pointers to the different problem constraints. This class owns the pointers.
	std::vector<AtomicFormula::cptr> _stateConstraints;
	std::vector<AtomicFormula::cptr> _goalConditions;

	//! The singleton instance
	static std::unique_ptr<Problem> _instance;
};

} // namespaces
