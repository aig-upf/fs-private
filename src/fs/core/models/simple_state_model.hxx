
#pragma once

#include <fs/core/actions/actions.hxx>
#include <fs/core/applicability//base.hxx>
#include <fs/core/atom.hxx>

// namespace lapkt { class MultivaluedState; }

namespace fs0 { namespace language { namespace fstrips { class AtomicFormula; }}}
namespace fs = fs0::language::fstrips;


namespace fs0 {

class Problem;
class State;

class SimpleStateModel { // : public aptk::DetStateModel<State, GroundAction> {
public:
	//using BaseT = aptk::DetStateModel<State, GroundAction>;
	using StateT = State;
// 	using StateT = lapkt::MultivaluedState;
	//using ActionType = BaseT::ActionType;
	using ActionType = GroundAction;
	using ActionId = ActionType::IdType;

	//! Factory method
	static SimpleStateModel build(const Problem& problem);

protected:
	SimpleStateModel(const Problem& problem, const std::vector<const fs::Formula*>& subgoals);

public:
	~SimpleStateModel() = default;

	SimpleStateModel(const SimpleStateModel&) =  delete;
	SimpleStateModel& operator=(const SimpleStateModel&) = delete;
	SimpleStateModel(SimpleStateModel&&) = default;
	SimpleStateModel& operator=(SimpleStateModel&&) = default;

	//! Returns initial state of the problem
	StateT init() const;

	//! Returns true if state is a goal state
	bool goal(const StateT& state) const;

	//! Returns applicable action set object
	GroundApplicableSet applicable_actions(const StateT& state, bool enforce_state_constraints) const;
	GroundApplicableSet applicable_actions(const StateT& state) const {
		return applicable_actions(state, true);
	}

	bool is_applicable(const StateT& state, const ActionType& action, bool enforce_state_constraints) const;
	bool is_applicable(const StateT& state, const ActionId& action, bool enforce_state_constraints) const;

	//! Returns the state resulting from applying the given action action on the given state
	StateT next(const StateT& state, const GroundAction::IdType& id) const;
	StateT next(const StateT& state, const GroundAction& a) const;

	//! Returns the number of subgoals into which the goal can be decomposed
	unsigned num_subgoals() const { return _subgoals.size(); }

	//! Returns true iff the given state satisfies the i-th subgoal
	bool goal(const StateT& s, unsigned i) const;

	const Problem& getTask() const { return _task; }

	unsigned get_action_idx(const ActionId& action) const { return static_cast<unsigned>(action); }

	static ActionManagerI* build_action_manager(const Problem& problem);

protected:
	// The underlying planning problem.
	const Problem& _task;

	std::unique_ptr<ActionManagerI> _manager;

	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;

	const std::vector<const fs::Formula*> _subgoals;
};

} // namespaces
