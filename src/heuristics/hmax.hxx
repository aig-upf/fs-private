
#pragma once

#include <heuristics/relaxed_plan.hxx>

namespace fs0 {

template <typename Model, typename RPGBuilder>
class HMaxHeuristic : public RelaxedPlanHeuristic<Model, RPGBuilder> {
public:

 	HMaxHeuristic(const Model& problem, std::vector<std::shared_ptr<BaseActionManager>>&& managers, std::shared_ptr<RPGBuilder> builder)
		: RelaxedPlanHeuristic<Model, RPGBuilder>(problem, std::move(managers), builder) {}
	virtual ~HMaxHeuristic() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg);
};

} // namespaces
