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

#pragma once

#include <aptk/search_prob.hxx>
#include <core_problem.hxx>
#include <state.hxx>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <applicable_action_set.hxx>

namespace aptk { namespace core {

/**
 *
 * 
 * NOTE: OF THE THREE POSSIBLE WAYS TO ITERATE THROUGH APPLICABLE ACTIONS
 * (NAMELY: (1) FwdSearchProblem::is_applicable, (2) FwdSearchProblem::applicable_set AND (3) Fwd_Search_Problem::Action_Iterator
 * ONLY (3) IS CURRENTLY USED BY THE SEARCH ENGINE.
 */
class FwdSearchProblem : public Search_Problem<State> {
	public:
		
		typedef aptk::core::ApplicableActionSet<aptk::core::SimpleActionSetManager> CoreApplicableActionSet; // To be referenced from consumers of the FwdSearchProblem class.

		FwdSearchProblem( const Problem& );
		virtual ~FwdSearchProblem();

		virtual	int      num_actions() const;
		
		virtual State*   init() const;
		
		virtual bool     goal( const State& s ) const;
		
// 		virtual bool     is_applicable( const aptk::core::State& s, aptk::core::ActionIdx idx ) const;
		
// 		virtual void     applicable_set( const aptk::core::State& s, std::vector< aptk::core::ActionIdx >& app_set ) const;	
		
		virtual float    cost( const State& s, Action_Idx a ) const;
		
		virtual State*   next( const State& s, Action_Idx a ) const;
		
		virtual void     print( std::ostream& os ) const;
		
		virtual CoreApplicableActionSet applicable_actions(const State& s) const;
		
		virtual const Problem& getTask() const { return task; }

	private:
		// The underlying core planning problem.
		const Problem& task;
};

} } // namespaces
