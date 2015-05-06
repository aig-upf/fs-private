
#pragma once

#include <aptk/search_prob.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <simple_applicable_action_set.hxx>

namespace fs0 {


class FwdSearchProblem : public aptk::Search_Problem<State> {
	public:
		
		typedef SimpleApplicableActionSet CoreApplicableActionSet; // To be referenced from consumers of the FwdSearchProblem class.

		FwdSearchProblem( const Problem& );
		virtual ~FwdSearchProblem();

		virtual	int      num_actions() const;
		
		virtual State*   init() const;
		
		virtual bool     goal( const State& s ) const;
		
 		virtual bool     is_applicable( const State& s, aptk::Action_Idx idx ) const;
		
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
