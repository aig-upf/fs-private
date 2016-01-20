
#pragma once

#include <memory>
#include <asp/model_handler.hxx>

namespace fs0 { class State; class Problem; }

namespace fs0 { namespace asp {

//! Computes the optimal (and admissible) h+ heuristic based on Clingo's optimization capabilities.
class ASPHPlus {
protected:
	//! The LP handler
	ModelHandler _handler;
	
public:
	ASPHPlus(const Problem& problem);

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
};

} } // namespaces
