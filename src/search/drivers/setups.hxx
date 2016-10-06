
#pragma once

#include <lifted_state_model.hxx>
#include <ground_state_model.hxx>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

//! A catalog of common setups for grounding actions for both search and heuristic computations.
class GroundingSetup {
public:
	static LiftedStateModel fully_lifted_model(Problem& problem);
	
	//! A simple model with all grounded actions
	static GroundStateModel fully_ground_model(Problem& problem);
	
	//! We'll use all the ground actions for the search plus the partially ground actions for the heuristic computations
	static GroundStateModel ground_search_lifted_heuristic(Problem& problem);

};

} } // namespaces
