
#pragma once

#include <fs0_types.hxx>


namespace fs0 {
	
class Problem; class State; class RelaxedState; class RPGData;

template <typename Model, typename RPGBuilder>
class RelaxedPlanHeuristic {
public:
	typedef typename Model::ActionType Action;

	RelaxedPlanHeuristic(const Model& problem, RPGBuilder* builder);

	virtual ~RelaxedPlanHeuristic() { delete _builder; }
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual float computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpgData);
	
protected:
	const Problem& _problem;
	
	const typename RPGBuilder::cptr _builder;
};

} // namespaces
