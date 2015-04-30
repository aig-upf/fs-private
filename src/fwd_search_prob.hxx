
#pragma once

#include <aptk/search_prob.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <simple_applicable_action_set.hxx>

namespace fs0 {

/**
 * 
 * NOTE: OF THE THREE POSSIBLE WAYS TO ITERATE THROUGH APPLICABLE ACTIONS
 * (NAMELY: (1) FwdSearchProblem::is_applicable, (2) FwdSearchProblem::applicable_set AND (3) Fwd_Search_Problem::Action_Iterator
 * ONLY (3) IS CURRENTLY USED BY THE SEARCH ENGINE.
 */
class FwdSearchProblem : public aptk::Search_Problem<State> {
	public:
		
		typedef SimpleApplicableActionSet CoreApplicableActionSet; // To be referenced from consumers of the FwdSearchProblem class.

		FwdSearchProblem( const Problem& );
		virtual ~FwdSearchProblem();

		virtual	int      num_actions() const;
		
		virtual State*   init() const;
		
		virtual bool     goal( const State& s ) const;
		
 		virtual bool     is_applicable( const State& s, aptk::Action_Idx idx ) const;
		
// 		virtual void     applicable_set( const aptk::core::State& s, std::vector< aptk::core::ActionIdx >& app_set ) const;	
		
		virtual float    cost( const State& s, aptk::Action_Idx a ) const;
		
		virtual State*   next( const State& s, aptk::Action_Idx a ) const;
		
		virtual void     print( std::ostream& os ) const;
		
		virtual CoreApplicableActionSet applicable_actions(const State& s) const;
		
		virtual const Problem& getTask() const { return task; }

	private:
		// The underlying core planning problem.
		const Problem& task;
};

} // namespaces
