
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
	
	void addGroundAction(const GroundAction::cptr action) { _ground.push_back(action); }
	const std::vector<GroundAction::cptr>& getGroundActions() const { return _ground; }	
	
	ApplicableActionSet getApplicableActions(const State& s) const;
	
	void registerStateConstraint(AtomicFormula::cptr constraint) { _stateConstraints.push_back(constraint);}
	const std::vector<AtomicFormula::cptr>& getStateConstraints() const { return _stateConstraints; }
	void registerGoalCondition(AtomicFormula::cptr condition) { _goalConditions.push_back(condition);}
	const std::vector<AtomicFormula::cptr>& getGoalConditions() const { return _goalConditions; }

	const std::string& get_action_name(unsigned action) const { return _problemInfo->getActionName(action); }

	//! Getter/setter for the associated ProblemInfo object.
	void setProblemInfo(ProblemInfo* info) { _problemInfo = info; }
	const ProblemInfo& getProblemInfo() const {
		if (!_problemInfo) throw std::runtime_error("ProblemInfo object has not been set yet");
		return *_problemInfo;
	}

	static void setCurrentProblem(Problem& problem) {
		_instance = &problem;
	}

	static const Problem* getCurrentProblem() {
		if (!_instance) throw std::runtime_error("Problem has not been instantiated yet");
		return _instance;
	}

	void compileConstraints();

	//! This performs a number of necessary routines once all of the problem information has been defined.
	void bootstrap();
	
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
	static const Problem* _instance;
};

} // namespaces
