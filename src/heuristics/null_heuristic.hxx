
#pragma once

namespace fs0 {

class State;

template < typename Model >
class NullHeuristic  {
public:
	typedef typename Model::ActionType Action;

	NullHeuristic( const Model& prob ) {}

	virtual ~NullHeuristic() {}

	float evaluate(const State& seed) { return 0.0f; }
};

} // namespaces
