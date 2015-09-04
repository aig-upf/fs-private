
#pragma once

#include <heuristics/relaxed_plan/constrained_relaxed_plan_heuristic.hxx>

namespace fs0 {

template <typename RPGBuilder>
class ConstrainedHMaxHeuristic : public ConstrainedRelaxedPlanHeuristic<RPGBuilder> {
public:

 	ConstrainedHMaxHeuristic(const FS0StateModel& model, std::vector<std::shared_ptr<BaseActionManager>>&& managers, std::shared_ptr<RPGBuilder> builder);
	virtual ~ConstrainedHMaxHeuristic() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg);
};

} // namespaces
