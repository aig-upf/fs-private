
#pragma once

#include <aptk2/search/interfaces/det_state_model.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <simple_applicable_action_set.hxx>

namespace fs0 {


class FwdSearchProblem : public aptk::DetStateModel<State, Action> {
	public:
			
		typedef SimpleApplicableActionSet CoreApplicableActionSet; // To be referenced from consumers of the FwdSearchProblem class.

		FwdSearchProblem( const Problem& );
		virtual ~FwdSearchProblem();

		//! Returns initial state of the problem
		virtual	State	init() const;
	
		//! Returns true if s is a goal state
		virtual bool	goal( const State& s ) const;
	
		//! Returns applicable action set object
		virtual	typename Action::ApplicableSet applicable_actions( const State& s ) const;
	
		//! Returns the state resulting from applying action a on state s
		virtual State	next( const State& s, typename Action::IdType id ) const;
		virtual State	next( const State& s, const Action& a ) const;

		// MRJ: Outputs a description of the problem
		virtual void 	print(std::ostream &os) const;
		
		virtual const Problem& getTask() const { return task; }

	private:
		// The underlying core planning problem.
		const Problem& task;
};

} // namespaces
