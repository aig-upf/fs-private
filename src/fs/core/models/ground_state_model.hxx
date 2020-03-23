
#pragma once

#include <lapkt/search/interfaces/det_state_model.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/applicability/base.hxx>

namespace fs0 {

class Problem;
class State;

class GroundStateModel { // : public aptk::DetStateModel<State, GroundAction> {
public:
	//using BaseT = aptk::DetStateModel<State, GroundAction>;
	using StateT = State;
	//using ActionType = BaseT::ActionType;
	using ActionType = GroundAction;
	using ActionId = ActionType::IdType;

	explicit GroundStateModel(const Problem& problem);
	~GroundStateModel() = default;

	GroundStateModel(const GroundStateModel&) = delete;
	GroundStateModel& operator=(const GroundStateModel&) = delete;
	GroundStateModel(GroundStateModel&&) = default;
	GroundStateModel& operator=(GroundStateModel&&) = delete;

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	GroundApplicableSet applicable_actions(const State& state, bool enforce_state_constraints) const;
	GroundApplicableSet applicable_actions(const State& state) const {
		return applicable_actions(state,true);
	}

//	bool is_applicable(const State& state, const ActionType& action, bool enforce_state_constraints) const;
//	bool is_applicable(const State& state, const ActionId& action, bool enforce_state_constraints) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const GroundAction::IdType& id) const;
	State next(const State& state, const GroundAction& a) const;

	const Problem& getTask() const { return _task; }

	static ActionManagerI* build_action_manager(const Problem& problem);

	const std::vector<Atom>& get_last_changeset() const {
		return _effects_cache;
	}

protected:
	// The underlying planning problem.
	const Problem& _task;

	std::unique_ptr<ActionManagerI> _manager;

	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
};

} // namespaces
