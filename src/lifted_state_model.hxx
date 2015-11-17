
#pragma once

#include <aptk2/search/interfaces/det_state_model.hxx>
#include <actions/action_id.hxx>


namespace fs0 { namespace gecode { class LiftedActionIterator; class ActionSchemaCSPHandler; }}

namespace fs0 {

class Problem;
class State;
class ActionSchema;
class GroundAction;


//! A state model that works with lifted actions instead of grounded actions
class LiftedStateModel : public aptk::DetStateModel<State, LiftedActionID> {
public:
	LiftedStateModel(const Problem& problem) : task(problem) {}
	~LiftedStateModel() {}

	//! Returns initial state of the problem
	State init() const;

	//! Returns true if state is a goal state
	bool goal(const State& state) const;

	//! Returns applicable action set object
	gecode::LiftedActionIterator applicable_actions(const State& state) const;

	//! Returns the state resulting from applying the given action action on the given state
	State next(const State& state, const LiftedActionID& action) const;
	State next(const State& state, const GroundAction& a) const;

	void print(std::ostream &os) const;
	
	const Problem& getTask() const { return task; }
	void set_handlers(std::vector<std::shared_ptr<gecode::ActionSchemaCSPHandler>>&& handlers) { _handlers = std::move(handlers); }

protected:
	// The underlying planning problem.
	const Problem& task;
	
	std::vector<std::shared_ptr<gecode::ActionSchemaCSPHandler>> _handlers;
};

} // namespaces
