
#pragma once

#include <heuristics/relaxed_plan.hxx>

namespace fs0 {

template <typename Model>
class HMaxHeuristic : public RelaxedPlanHeuristic<Model> {
public:

 	HMaxHeuristic(const Model& problem) : RelaxedPlanHeuristic<Model>(problem) {}
	virtual ~HMaxHeuristic() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg) {
		if (this->_problem.getConstraintManager()->isGoal(state)) {
			return rpg.getNumLayers();
		}
		return -1;
	}
};

} // namespaces
