
#pragma once

#include <fs/core/actions/csp_action_iterator.hxx>
#include <fs/core/actions/simple_lifted_operators.hxx>
#include <fs/core/constraints/gecode/v2/extensions.hxx>


namespace fs0::gecode { class LiftedActionCSP; }
namespace fs0::gecode::v2 { class ActionSchemaCSP; }

namespace fs0 {

class LiftedActionID;
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
    CSPLiftedStateModel(
            const Problem& problem,
            std::vector<const fs::Formula*> subgoals,
            std::vector<const PartiallyGroundedAction*>&& schemas,
            std::vector<SimpleLiftedOperator>&& lifted_operators,
            std::vector<gecode::v2::ActionSchemaCSP>&& schema_csps,
            gecode::v2::SymbolExtensionGenerator&& extension_generator);

public:

	//! Factory method
	static CSPLiftedStateModel build(const Problem& problem, const ProblemInfo& info, const AtomIndex& atom_index);

	~CSPLiftedStateModel();

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

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const ActionType& aid) const;

	const Problem& getTask() const { return problem; }

	//! Returns the number of subgoals into which the goal can be decomposed
	unsigned num_subgoals() const { return _subgoals.size(); }

	//! Returns true iff the given state satisfies the i-th subgoal
	bool goal(const StateT& s, unsigned i) const;

	const std::vector<Atom>& get_last_changeset() const {
		return _effects_cache;
	}

protected:
	// The underlying planning problem.
	const Problem& problem;

    const std::vector<const fs::Formula*> _subgoals;

    std::vector<const PartiallyGroundedAction*> schemas;

    std::vector<SimpleLiftedOperator> lifted_operators;

    std::vector<gecode::v2::ActionSchemaCSP> schema_csps;

    gecode::v2::SymbolExtensionGenerator extension_generator;

	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
};

} // namespaces
