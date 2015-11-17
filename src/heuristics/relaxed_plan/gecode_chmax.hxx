
#pragma once

#include <heuristics/relaxed_plan/gecode_crpg.hxx>

namespace fs0 { namespace gecode {

class GecodeCHMax : public GecodeCRPG {
public:

	GecodeCHMax(const Problem& problem, std::vector<std::shared_ptr<BaseActionCSPHandler>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	virtual ~GecodeCHMax() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	long computeHeuristic(const State& seed, const GecodeRPGLayer& state, const RPGData& bookkeeping);

};

} } // namespaces
