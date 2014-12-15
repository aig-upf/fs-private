
#pragma once

#include <state.hxx>
#include <core_problem.hxx>
#include <constraints/csp_goal_manager.hxx>
#include <heuristics/relaxed_plan.hxx>

namespace aptk { namespace core {

template <typename Model>
class HMaxHeuristic : public RelaxedPlanHeuristic<Model> {
public:

 	HMaxHeuristic(const Model& problem) : RelaxedPlanHeuristic<Model>(problem) {}
	virtual ~HMaxHeuristic() {}
	
	//! The hmax heuristic only cares about the size of the RP graph.
	float computeHeuristic(const State& seed, const RelaxedState::ptr& s1, const std::vector<Changeset::ptr>& changesets) {
		return (this->_goal_manager).isGoal(seed, *s1) ? changesets.size() : -1;
	}
};

} } // namespaces
