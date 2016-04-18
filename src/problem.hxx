
#pragma once

#include <fs_types.hxx>
#include <utils/tuple_index.hxx>

namespace fs0 { namespace asp { class LPHandler; }}
namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class State;
class FormulaInterpreter;
class ActionData;
class ActionBase;
class PartiallyGroundedAction;
class GroundAction;

class Problem {
public:

	Problem(State* init, const std::vector<const ActionData*>& action_data, const fs::Formula* goal, const fs::Formula* state_constraints, TupleIndex&& tuple_index);
	~Problem();
	
	Problem(const Problem& other) = delete;
	Problem& operator=(const Problem& other) = delete;
	Problem(Problem&& other) = delete;
	bool operator==(const Problem& other) = delete;
	Problem& operator=(Problem&& rhs) = delete;

	//! Get the initial state of the problem
	const State& getInitialState() const { return *_init; }

	//! Get the set of action schemata of the problem
	const std::vector<const ActionData*>& getActionData() const { return _action_data; }
	
	//! Get the set of ground actions of the problem
	const std::vector<const GroundAction*>& getGroundActions() const { return _ground; }
	void setGroundActions(std::vector<const GroundAction*>&& ground) { _ground = std::move(ground); }
	
	const std::vector<const PartiallyGroundedAction*>& getPartiallyGroundedActions() const { return _partials; }
	void setPartiallyGroundedActions(std::vector<const PartiallyGroundedAction*>&& actions) { _partials = std::move(actions); }
	
	//! Get the problem's goal formula
	const fs::Formula* getGoalConditions() const { return _goal_formula; }
	
	//! Get the state constraint formula of the problem
	const fs::Formula* getStateConstraints() const { return _state_constraint_formula; }
	
	const FormulaInterpreter& getGoalSatManager() const { return *_goal_sat_manager; }
	
	void setLPHandler(asp::LPHandler* handler) { _lp_handler = handler; }
	const asp::LPHandler* getLPHandler() const { return _lp_handler; }

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
	
	const TupleIndex& get_tuple_index() const { return _tuple_index; }
	
	//! Return true if all the symbols of the problem are predicates
	bool is_predicative() const { return _is_predicative; }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Problem& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	//! An index of tuples and atoms
	TupleIndex _tuple_index;
	
	//! The initial state of the problem
	const std::unique_ptr<State> _init;

	const std::vector<const ActionData*> _action_data;
	
	// The set of grounded actions of the problem
	std::vector<const GroundAction*> _ground;
	
	// The possible set of partially grounded actions of the problem
	std::vector<const PartiallyGroundedAction*> _partials;
	
	//! Pointers to the goal and state constraints formulas. This class owns the pointers.
	const fs::Formula* _state_constraint_formula;
	const fs::Formula* _goal_formula;

	std::unique_ptr<FormulaInterpreter> _goal_sat_manager;
	
	asp::LPHandler* _lp_handler = nullptr;
	
	//! Whether all the symbols of the problem are predicates
	const bool _is_predicative;
	
	//! The singleton instance
	static std::unique_ptr<Problem> _instance;
	
	static bool check_is_predicative();
};

} // namespaces
