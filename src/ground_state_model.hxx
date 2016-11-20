
#pragma once

#include <aptk2/search/interfaces/det_state_model.hxx>
#include <actions/actions.hxx>
#include <applicability/action_managers.hxx>
#include "atom.hxx"

namespace fs0 {

class Problem;
class State;

class GroundStateModel : public aptk::DetStateModel<State, GroundAction> {
public:
	using BaseT = aptk::DetStateModel<State, GroundAction>;
	using ActionType = BaseT::ActionType;
	using ActionId = ActionType::IdType;
	
	GroundStateModel(const Problem& problem, BasicApplicabilityAnalyzer* analyzer = nullptr, bool remove_trajectory = false);
	~GroundStateModel() = default;
	
	GroundStateModel(const GroundStateModel&) = default;
	GroundStateModel& operator=(const GroundStateModel&) = default;
	GroundStateModel(GroundStateModel&&) = default;
	GroundStateModel& operator=(GroundStateModel&&) = default;

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	GroundApplicableSet applicable_actions(const State& state) const;
	
	bool is_applicable(const State& state, const ActionType& action) const;
	bool is_applicable(const State& state, const ActionId& action) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const GroundAction::IdType& id) const;
	State next(const State& state, const GroundAction& a) const;

	void print(std::ostream &os) const;
	
	const Problem& getTask() const { return _task; }

protected:
	// The underlying planning problem.
	const Problem& _task;
	
	const SmartActionManager _manager;
	
	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
	
	static SmartActionManager build_action_manager(const Problem& problem, BasicApplicabilityAnalyzer* analyzer);

	fs0::Atom _null_trajectory_atom;
	//! HACK HACK HACK
	std::vector<fs0::Atom> _null_trajectory;
	
	bool _remove_trajectory;

	void nullify_trajectory(State& state) const;
};

} // namespaces
