
#pragma once

#include <fs/core/atom.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/actions/csp_action_iterator.hxx>


namespace fs0::gecode { class LiftedActionCSP; }

namespace fs0 {

class Problem;
class State;
class GroundAction;


//! A state model that works with lifted actions instead of grounded actions
class CSPLiftedStateModel
{
public:
	using StateT = State;
	using ActionType = LiftedActionID;

protected:
	CSPLiftedStateModel(const Problem& problem, const std::vector<const fs::Formula*>& subgoals);

public:

	//! Factory method
	static CSPLiftedStateModel build(const Problem& problem);

	~CSPLiftedStateModel() = default;

	CSPLiftedStateModel(const CSPLiftedStateModel&) = default;
	CSPLiftedStateModel& operator=(const CSPLiftedStateModel&) = delete;
	CSPLiftedStateModel(CSPLiftedStateModel&&) = default;
	CSPLiftedStateModel& operator=(CSPLiftedStateModel&&) = delete;

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	gecode::CSPActionIterator applicable_actions(const State& state, bool enforce_state_constraints) const;
	gecode::CSPActionIterator applicable_actions(const State& state) const {
		return applicable_actions(state, true);
	}

	bool is_applicable(const State& state, const GroundAction& action, bool enforce_state_constraints) const;
	bool is_applicable(const State& state, const ActionType& action, bool enforce_state_constraints) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const ActionType& aid) const;

	const Problem& getTask() const { return _task; }
	void set_handlers(std::vector<std::shared_ptr<gecode::LiftedActionCSP>>&& handlers) { _handlers = std::move(handlers); }

	//! Returns the number of subgoals into which the goal can be decomposed
	unsigned num_subgoals() const { return _subgoals.size(); }

	//! Returns true iff the given state satisfies the i-th subgoal
	bool goal(const StateT& s, unsigned i) const;

	const std::vector<Atom>& get_last_changeset() const {
		return _effects_cache;
	}

protected:
	// The underlying planning problem.
	const Problem& _task;

	std::vector<std::shared_ptr<gecode::LiftedActionCSP>> _handlers;

	const std::vector<const fs::Formula*> _subgoals;

	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
};

} // namespaces
