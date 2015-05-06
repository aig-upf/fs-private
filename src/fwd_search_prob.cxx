
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

// We implement this only to make the class non-abstract, but should not be using it.
bool FwdSearchProblem::is_applicable( const State& s, aptk::Action_Idx idx ) const {
	throw std::runtime_error("Should not be using this method but the action iterator instead");
}


float	FwdSearchProblem::cost( const State& s, aptk::Action_Idx a ) const {
	return 1.0; // TODO - Implement if support for action costs is needed
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

