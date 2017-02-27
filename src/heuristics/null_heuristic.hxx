
#pragma once

#include <state.hxx>
#include <actions/actions.hxx>
#include <models/ground_state_model.hxx>

namespace fs0 {

class NullHeuristic  {
public:
	typedef GroundAction Action;

	NullHeuristic( const GroundStateModel& prob ) {}

	float evaluate(const State& seed) { return 0.0f; }
};

} // namespaces
