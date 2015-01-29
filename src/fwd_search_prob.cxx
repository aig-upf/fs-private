
#include <fwd_search_prob.hxx>
#include <algorithm>
#include <action_manager.hxx>

namespace fs0 {

FwdSearchProblem::FwdSearchProblem( const Problem& p )
	: task( p )
	{}

FwdSearchProblem::~FwdSearchProblem() {}

int FwdSearchProblem::num_actions() const { return task.getNumActions(); }

State* FwdSearchProblem::init() const {
	// We need to make a copy so that we can return it as non-const.
	// Ugly, but this way we make it fit the search engine interface without further changes,
	// and this is only called once per search.
	return new State(*task.getInitialState());
}

bool FwdSearchProblem::goal( const State& s ) const { return task.isGoal(s); }

// bool FwdSearchProblem::is_applicable( const State& s, ActionIdx idx ) const {
// 	return task.getAction(idx)->applicable(s);
// }

// THIS FUNCTION IS CURRENTLY NOT USED IN THE ENGINE, AND IT'S NOT STRAIGHT-FORWARD TO IMPLEMENT, SINCE IT
// WAS DESIGNED FOR GROUNDED ACTIONS, WHICH I AM TRYING TO AVOID. THUS WE'LL LEFT IT UNIMPLEMENTED.
// void FwdSearchProblem::applicable_set( const State& s, std::vector<ActionIdx>& app_set ) const {
// 	throw NonImplementedException(); 
// 	// m_task->applicable_actions( s, app_set ); 
// }

float	FwdSearchProblem::cost( const State& s, aptk::Action_Idx a ) const {
// 	const Action& act = *(task.actions().at(a));
// 	return act.cost();
  return 1.0; // TODO - Implement
}

State* FwdSearchProblem::next( const State& s, aptk::Action_Idx actionIdx ) const {
	StandardApplicabilityManager manager(s, task.getConstraints());
	return new State(s, manager.computeEffects(*task.getAction(actionIdx))); // Copy everything into the new state and apply the changeset
} 

void FwdSearchProblem::print( std::ostream& os ) const {
	os << "[NON-IMPLEMENTED]";
}

FwdSearchProblem::CoreApplicableActionSet FwdSearchProblem::applicable_actions(const State& s) const {
	return task.getApplicableActions(s);
}
		
} // namespaces

