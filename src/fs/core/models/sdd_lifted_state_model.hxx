
#pragma once

#include <fs/core/atom.hxx>
#include <fs/core/actions/sdd_action_iterator.hxx>
#include <fs/core/actions/propositional_actions.hxx>
#include <fs/core/utils/sdd.hxx>


namespace fs0::gecode { class LiftedActionCSP; }

namespace fs0 {

class Problem;
class State;
class GroundAction;


//! A state model that works with lifted actions instead of grounded actions
class SDDLiftedStateModel
{
public:
	using StateT = State;
	using ActionType = SchematicActionID;

protected:
	SDDLiftedStateModel(const Problem& problem, std::vector<std::shared_ptr<ActionSchemaSDD>> sdds, std::vector<const fs::Formula*> subgoals);

public:

	//! Factory method
	static SDDLiftedStateModel build(const Problem& problem);

	~SDDLiftedStateModel() = default;

	SDDLiftedStateModel(const SDDLiftedStateModel&) = default;
	SDDLiftedStateModel& operator=(const SDDLiftedStateModel&) = delete;
	SDDLiftedStateModel(SDDLiftedStateModel&&) = default;
	SDDLiftedStateModel& operator=(SDDLiftedStateModel&&) = delete;

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	SDDActionIterator applicable_actions(const State& state) const;
    SDDActionIterator applicable_actions(const State& state, bool enforce_state_constraints) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const SchematicActionID& action) const;

	const Problem& getTask() const { return _task; }

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

    std::vector<std::shared_ptr<ActionSchemaSDD>> sdds_;

	const std::vector<const fs::Formula*> _subgoals;

	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
};

} // namespaces
