
#pragma once

#include <state.hxx>
#include <actions/ground_action.hxx>
#include <state_model.hxx>

namespace fs0 {

class NullHeuristic  {
public:
	typedef GroundAction Action;

	NullHeuristic( const FS0StateModel& prob ) {}

	float evaluate(const State& seed) { return 0.0f; }
};

} // namespaces
