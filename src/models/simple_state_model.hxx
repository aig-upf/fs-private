
#pragma once

#include <aptk2/search/interfaces/det_state_model.hxx>
#include <actions/actions.hxx>
#include <applicability/action_managers.hxx>
#include <atom.hxx>

// namespace lapkt { class MultivaluedState; }

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
	static SimpleStateModel build(const Problem& problem, BasicApplicabilityAnalyzer* analyzer = nullptr);
	
protected:
	SimpleStateModel(const Problem& problem, std::vector<Atom> subgoals, BasicApplicabilityAnalyzer* analyzer = nullptr);

public:
	~SimpleStateModel();

	SimpleStateModel(const SimpleStateModel&) = default;
	SimpleStateModel& operator=(const SimpleStateModel&) = default;
	SimpleStateModel(SimpleStateModel&&) = default;
	SimpleStateModel& operator=(SimpleStateModel&&) = default;

	//! Returns initial state of the problem
	StateT init() const;

	//! Returns true if state is a goal state
	bool goal(const StateT& state) const;

	//! Returns applicable action set object
	GroundApplicableSet applicable_actions(const StateT& state) const;

	bool is_applicable(const StateT& state, const ActionType& action) const;
	bool is_applicable(const StateT& state, const ActionId& action) const;

	//! Returns the state resulting from applying the given action action on the given state
	StateT next(const StateT& state, const GroundAction::IdType& id) const;
	StateT next(const StateT& state, const GroundAction& a) const;

	//! Returns the number of subgoals into which the goal can be decomposed
	unsigned num_subgoals() const { return _subgoals.size(); }

	//! Returns true iff the given state satisfies the i-th subgoal
	bool goal(const StateT& s, unsigned i) const;

	const Problem& getTask() const { return _task; }

	unsigned get_action_idx(const ActionId& action) const { return static_cast<unsigned>(action); }

protected:
	// The underlying planning problem.
	const Problem& _task;

	const SmartActionManager* _manager;

	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;

	static SmartActionManager* build_action_manager(const Problem& problem, BasicApplicabilityAnalyzer* analyzer);
	
	std::vector<Atom> _subgoals;
};

} // namespaces
