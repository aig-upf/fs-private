
#pragma once

#include <fs_types.hxx>

namespace fs0 {

class State;
class Problem;
class LiftedActionID;
class GroundAction;

class Checker {
public:
	//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
	static bool check_correctness(const Problem& problem, const ActionPlan& plan, const State& s0);
	
	//! Returns true iff the given lifted-action plan is valid and leads to a goal state when applied to s0
	static bool check_correctness(const Problem& problem, const std::vector<LiftedActionID>& plan, const State& s0);
};


} // namespaces
