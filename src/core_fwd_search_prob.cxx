/*
Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <core_fwd_search_prob.hxx>
#include <algorithm>
#include <action_manager.hxx>

namespace aptk { namespace core {

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

float	FwdSearchProblem::cost( const State& s, Action_Idx a ) const {
// 	const Action& act = *(task.actions().at(a));
// 	return act.cost();
  return 1.0; // TODO - Implement
}

State* FwdSearchProblem::next( const State& s, Action_Idx actionIdx ) const {
	Changeset changeset;
	SimpleActionSetManager manager(s, task.getConstraints());
	manager.computeChangeset(*task.getAction(actionIdx), changeset);
	return new State(s, changeset); // Copy everything into the new state and apply the changeset
} 

void FwdSearchProblem::print( std::ostream& os ) const {
	os << "[NON-IMPLEMENTED]";
}

FwdSearchProblem::CoreApplicableActionSet FwdSearchProblem::applicable_actions(const State& s) const {
	return task.getApplicableActions(s);
}
		
} } // namespaces

