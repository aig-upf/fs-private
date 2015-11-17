
#pragma once

#include <aptk2/search/interfaces/det_state_model.hxx>
#include <actions/ground_action.hxx>

namespace fs0 {

class Problem;
class State;

class FS0StateModel : public aptk::DetStateModel<State, GroundAction> {
public:
	FS0StateModel(const Problem& problem) : task(problem) {}
	~FS0StateModel() {}

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	typename GroundAction::ApplicableSet applicable_actions(const State& state) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const GroundAction::IdType& id) const;
	State next(const State& state, const GroundAction& a) const;

	void print(std::ostream &os) const;
	
	const Problem& getTask() const { return task; }

protected:
	// The underlying planning problem.
	const Problem& task;
};

} // namespaces
