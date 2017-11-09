
#pragma once

#include <fs/core/state.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/models/ground_state_model.hxx>

namespace fs0 {

class NullHeuristic  {
public:
	typedef GroundAction Action;

	NullHeuristic( const GroundStateModel& prob ) {}

	float evaluate(const State& seed) { return 0.0f; }
};

} // namespaces
