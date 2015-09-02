
#pragma once

#include <fs0_types.hxx>


namespace fs0 {
	
class Problem; class State; class RelaxedState; class RPGData; class BaseActionManager;

template <typename Model, typename RPGBuilder>
class RelaxedPlanHeuristic {
public:
	typedef typename Model::ActionType Action;

	RelaxedPlanHeuristic(const Model& problem, std::vector<std::shared_ptr<BaseActionManager>>&& managers, std::shared_ptr<RPGBuilder> builder);
	
	virtual ~RelaxedPlanHeuristic() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpgData);
	
protected:
	//! The actual planning problem
	const Problem& _problem;
	
	//! The set of action managers, one per every action
	const std::vector<std::shared_ptr<BaseActionManager>> _managers;
	
	//! The RPG building helper
	const std::shared_ptr<RPGBuilder> _builder;
};

} // namespaces
