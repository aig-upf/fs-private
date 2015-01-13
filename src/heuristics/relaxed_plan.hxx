
#pragma once

#include <vector>
#include <algorithm>

#include <aptk/heuristic.hxx>
#include <state.hxx>
#include <core_problem.hxx>
#include <heuristics/changeset.hxx>


namespace aptk { namespace core {

class RPGraph;

template < typename SearchModel >
class RelaxedPlanHeuristic : public Heuristic<State> {
public:

	RelaxedPlanHeuristic( const SearchModel& problem );

	virtual ~RelaxedPlanHeuristic() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual float computeHeuristic(RPGraph& rpg, const State& seed, const RelaxedState& state, const Changeset::vptr& changesets);
	
	//! A helper to print information about the RPG changesets.
	void print_changesets(const std::vector<Changeset::ptr>& changesets);
	
	//! Proxy to circumvent the unusual virtual method signature
	virtual void eval(const State& s, float& h_val) { h_val = evaluate(s); }
	
	//! So far just act as a proxy, we do not compute the preferred operations yet.
	virtual void eval( const State& s, float& h_val,  std::vector<Action_Idx>& pref_ops ) { eval(s, h_val); }
	
protected:
	const Problem& _problem;
};

} } // namespaces
