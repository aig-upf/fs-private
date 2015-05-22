
#pragma once

#include <state.hxx>

namespace fs0 {

template < typename ProblemT >
class NullHeuristic  {
public:

	typedef typename ProblemT::ActionType		Action;	
	typedef std::vector< typename Action::IdType >	PrefOpsVec;

	NullHeuristic( const ProblemT& prob )
	{}

	virtual ~NullHeuristic() {}

	virtual void eval( const State& s, float& h_val );
	virtual void eval( const State& s, float& h_val, PrefOpsVec& pref_ops );
};

} // namespaces
