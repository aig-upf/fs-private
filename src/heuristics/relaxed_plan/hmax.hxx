
#pragma once

#include <heuristics/relaxed_plan/constrained_relaxed_plan_heuristic.hxx>

namespace fs0 {

template <typename RPGBuilder>
class HMaxHeuristic : public ConstrainedRelaxedPlanHeuristic<RPGBuilder> {
public:

 	HMaxHeuristic(const FS0StateModel& model, std::vector<std::shared_ptr<BaseActionManager>>&& managers, std::shared_ptr<RPGBuilder> builder);
	virtual ~HMaxHeuristic() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg);
};

} // namespaces
