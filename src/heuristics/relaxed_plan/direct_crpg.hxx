
#pragma once

#include <fs0_types.hxx>
#include <constraints/direct/action_manager.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>

namespace fs0 {

class GroundAction; class Problem; class State; class RelaxedState;

class DirectCRPG {
public:
	typedef GroundAction Action;

	DirectCRPG(const Problem& problem, std::vector<std::shared_ptr<DirectActionManager>>&& managers, std::shared_ptr<DirectRPGBuilder> builder);
	
	virtual ~DirectCRPG() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
	
	//! A version where only certain actions are allowed
	long evaluate(const State& seed, const std::vector<ActionIdx>& whitelist);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& bookkeeping);
	
protected:
	//! The actual planning problem
	const Problem& _problem;
	
	//! The set of action managers, one per every action
	const std::vector<std::shared_ptr<DirectActionManager>> _managers;
	
	//! A whitelist including all possible actions
	std::vector<unsigned> all_whitelist;
	
	//! The RPG building helper
	const std::shared_ptr<DirectRPGBuilder> _builder;
};

} // namespaces
