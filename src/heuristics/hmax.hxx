
#pragma once

#include <heuristics/relaxed_plan.hxx>

namespace fs0 {

template <typename Model, typename RPGBuilder>
class HMaxHeuristic : public RelaxedPlanHeuristic<Model, RPGBuilder> {
public:

 	HMaxHeuristic(const Model& problem, RPGBuilder::cptr builder)
		: RelaxedPlanHeuristic<Model, RPGBuilder>(problem, builder) {}
	virtual ~HMaxHeuristic() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg) {
		if (_builder->isGoal(state)) return rpg.getCurrentLayerIdx();
		return -1;
	}
};

} // namespaces
