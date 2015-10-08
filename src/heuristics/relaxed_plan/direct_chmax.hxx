
#pragma once

#include <heuristics/relaxed_plan/direct_crpg.hxx>

namespace fs0 {

class DirectCHMax : public DirectCRPG {
public:

 	DirectCHMax(const FS0StateModel& model, std::vector<std::shared_ptr<DirectActionManager>>&& managers, std::shared_ptr<DirectRPGBuilder> builder);
	virtual ~DirectCHMax() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	long computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& bookkeeping);
};

} // namespaces
