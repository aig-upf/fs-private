
#include <fwd_search_prob.hxx>
#include <algorithm>
#include <action_manager.hxx>

namespace fs0 {

FwdSearchProblem::FwdSearchProblem( const Problem& p )
	: task( p )
	{}

FwdSearchProblem::~FwdSearchProblem() {}

State FwdSearchProblem::init() const {
	// We need to make a copy so that we can return it as non-const.
	// Ugly, but this way we make it fit the search engine interface without further changes,
	// and this is only called once per search.
	return State(*task.getInitialState());
}

bool FwdSearchProblem::goal( const State& s ) const { return task.isGoal(s); }

State FwdSearchProblem::next( const State& s, typename Action::IdType actionIdx ) const {
	StandardApplicabilityManager manager(s, task.getConstraints());
	return State(s, manager.computeEffects(*task.getAction(actionIdx))); // Copy everything into the new state and apply the changeset
} 

State FwdSearchProblem::next( const State& s, const Action& a ) const { 
	StandardApplicabilityManager manager(s, task.getConstraints());
	return State(s, manager.computeEffects(a)); // Copy everything into the new state and apply the changeset
}


void FwdSearchProblem::print( std::ostream& os ) const {
	os << "[NOT-IMPLEMENTED]";
}

typename Action::ApplicableSet
FwdSearchProblem::applicable_actions(const State& s) const {
	return task.getApplicableActions(s);
}
		
} // namespaces

