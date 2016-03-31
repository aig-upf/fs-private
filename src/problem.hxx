
#pragma once

#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <utils/tuple_index.hxx>

namespace fs0 { namespace asp { class LPHandler; }}
namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ActionSchema; class GroundAction; class State; class FormulaInterpreter;
class ApplicableActionSet; class ProblemInfo;

class Problem {
public:

	Problem(State* init, const std::vector<const ActionSchema*>& schemata, const fs::Formula* goal, const fs::Formula* state_constraints, TupleIndex&& tuple_index);
	~Problem();

	//! Get the initial state of the problem
	const State& getInitialState() const { return *_init; }

	//! Get the set of action schemata of the problem
	const std::vector<const ActionSchema*>& getActionSchemata() const { return _schemata; }
	
	//! Get the set of ground actions of the problem
	const std::vector<const GroundAction*>& getGroundActions() const { return _ground; }
	void setGroundActions(std::vector<const GroundAction*>&& ground) { _ground = std::move(ground); }
	
	//! Get an iterator on the set of actions that are applicable in the given state
	ApplicableActionSet getApplicableActions(const State& s) const;
	
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
	
	//! Get the ProblemInfo object associated to this problem.
	const ProblemInfo& getProblemInfo() const { return getInfo(); }
	
	static void setInfo(ProblemInfo* info) {
		assert(!_info);
		_info = std::unique_ptr<ProblemInfo>(info);
	}
	
	static const ProblemInfo& getInfo() {
		assert(_info);
		return *_info;
	}
	
	const TupleIndex& get_tuple_index() const { return _tuple_index; }


	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Problem& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	//! An index of tuples and atoms
	TupleIndex _tuple_index;
	
	//! The initial state of the problem
	const std::unique_ptr<State> _init;

	// The set of action schemata
	const std::vector<const ActionSchema*> _schemata;
	
	// The set of grounded actions of the problem
	std::vector<const GroundAction*> _ground;
	
	//! Pointers to the goal and state constraints formulas. This class owns the pointers.
	const fs::Formula* _state_constraint_formula;
	const fs::Formula* _goal_formula;

	std::unique_ptr<FormulaInterpreter> _goal_sat_manager;
	
	asp::LPHandler* _lp_handler = nullptr;
	
	//! The singleton instance
	static std::unique_ptr<Problem> _instance;
	
	//! An object with all sorts of extra book-keeping information - currently a singleton instance as well :-(
	static std::unique_ptr<ProblemInfo> _info;
};

} // namespaces
