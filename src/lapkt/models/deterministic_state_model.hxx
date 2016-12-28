
#pragma once

#include <iostream>

namespace lapkt {

template <typename _StateT, typename _ActionIdT, typename _ApplicableSetT> 
class DeterministicStateModel {
public:
// 	const typename ActionT::IdType invalid_action = ActionT::invalid_action_id; // TODO Remove?
	using StateT = _StateT;
	using ActionIdT = _ActionIdT;
	using ApplicableSetT = _ApplicableSetT;
	using ValueT = typename StateT::ValueT;

	DeterministicStateModel() = default;
	virtual ~DeterministicStateModel() = default;

	//! Returns s_0, initial state of the problem
	virtual StateT initial() const = 0;

	//! Returns A(s), the set of actions applicable in a given state
	virtual ApplicableSetT applicable(const StateT&) const = 0;

	//! Returns f_a(s), the state resulting from applying action a on state s
	virtual StateT next(const StateT& s, const ActionIdT& a) const = 0;

	//! Returns true iff the given state is a goal state
	virtual bool goal(const StateT& s) const = 0;

	//! Returns the number of subgoals into which the goal can be decomposed
	virtual unsigned num_subgoals() const = 0;

	//! Returns true iff the given state satisfies the i-th subgoal
	virtual bool goal(const StateT& s, unsigned i) const = 0;

	//! Returns the number of variables into which any problem state can be factorized
	// virtual unsigned num_state_variables() const = 0;

	//! Returns the number of variables into which any problem state can be factorized
	// virtual ValueT value(const StateT& s, unsigned i) const = 0;
};

} // namespaces
