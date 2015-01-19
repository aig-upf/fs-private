
#pragma once

#include <aptk/heuristic.hxx>
#include <state.hxx>

namespace fs0 {

template < typename ProblemT >
class NullHeuristic : public aptk::Heuristic<State> {
public:
	
	NullHeuristic( const ProblemT& prob ) :
		Heuristic<State>( prob ) {}

	virtual ~NullHeuristic() {}

	virtual void eval( const State& s, float& h_val );
	virtual void eval( const State& s, float& h_val,  std::vector<aptk::Action_Idx>& pref_ops );
};

} // namespaces
