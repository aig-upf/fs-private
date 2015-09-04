
#pragma once

#include <fs0_types.hxx>

namespace fs0 {

class GroundAction; class FS0StateModel; class Problem; class State; class RelaxedState; class RPGData; class BaseActionManager;

template <typename RPGBuilder>
class ConstrainedRelaxedPlanHeuristic {
public:
	typedef GroundAction Action;

	ConstrainedRelaxedPlanHeuristic(const FS0StateModel& model, std::vector<std::shared_ptr<BaseActionManager>>&& managers, std::shared_ptr<RPGBuilder> builder);
	
	virtual ~ConstrainedRelaxedPlanHeuristic() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpgData);
	
protected:
	//! The actual planning problem
	const Problem& _problem;
	
	//! The set of action managers, one per every action
	const std::vector<std::shared_ptr<BaseActionManager>> _managers;
	
	//! The RPG building helper
	const std::shared_ptr<RPGBuilder> _builder;
};

} // namespaces
