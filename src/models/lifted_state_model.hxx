
#pragma once

#include <actions/action_id.hxx>
#include <actions/lifted_action_iterator.hxx>


namespace fs0 { namespace gecode { class LiftedActionCSP; }}

namespace fs0 {

class Problem;
class State;
class GroundAction;


//! A state model that works with lifted actions instead of grounded actions
class LiftedStateModel
{
public:
	using StateT = State;
	using ActionType = LiftedActionID;
	
protected:
	LiftedStateModel(const Problem& problem, const std::vector<const fs::Formula*>& subgoals);
	
public:
	
	//! Factory method
	static LiftedStateModel build(const Problem& problem);

	~LiftedStateModel() = default;
	
	LiftedStateModel(const LiftedStateModel&) = default;
	LiftedStateModel& operator=(const LiftedStateModel&) = default;
	LiftedStateModel(LiftedStateModel&&) = default;
	LiftedStateModel& operator=(LiftedStateModel&&) = default;

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	gecode::LiftedActionIterator applicable_actions(const State& state) const;
	
	bool is_applicable(const State& state, const GroundAction& action) const;
	bool is_applicable(const State& state, const ActionType& action) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const ActionType& action) const;
	State next(const State& state, const GroundAction& a) const;

	
	const Problem& getTask() const { return _task; }
	void set_handlers(std::vector<std::shared_ptr<gecode::LiftedActionCSP>>&& handlers) { _handlers = std::move(handlers); }
	
	unsigned get_action_idx(const LiftedActionID& action) const { return 0; }
	
	//! Returns the number of subgoals into which the goal can be decomposed
	unsigned num_subgoals() const { return _subgoals.size(); }
	
	//! Returns true iff the given state satisfies the i-th subgoal
	bool goal(const StateT& s, unsigned i) const;	

protected:
	// The underlying planning problem.
	const Problem& _task;
	
	std::vector<std::shared_ptr<gecode::LiftedActionCSP>> _handlers;
	
	const std::vector<const fs::Formula*> _subgoals;
};

} // namespaces
