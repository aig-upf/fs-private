
#pragma once

#include <fs/core/fs_types.hxx>

namespace fs0 {

class State;
class Problem;
class LiftedActionID;
class GroundAction;

class Checker {
public:
	//! Transform different plan formats into a vector of ground actions.
	static std::vector<const GroundAction*> transform(const Problem& problem, const std::vector<LiftedActionID>& plan);
	static std::vector<const GroundAction*> transform(const Problem& problem, const ActionPlan& plan);
	
	static bool check_correctness(const Problem& problem, const std::vector<const GroundAction*>& plan, const State& s0);

	
	//! Returns true iff the given plan is valid AND leads to a goal state, when applied to state s0.
	static bool check_correctness(const Problem& problem, const ActionPlan& plan, const State& s0) {
		return check_correctness(problem, transform(problem, plan), s0);
	}
	
	//! Returns true iff the given lifted-action plan is valid and leads to a goal state when applied to s0
	static bool check_correctness(const Problem& problem, const std::vector<LiftedActionID>& plan, const State& s0) {
		return check_correctness(problem, transform(problem, plan), s0);
	}

	static void print_plan_execution(const Problem& problem, const std::vector<const GroundAction*>& plan, const State& s0);


	static void print_plan_execution(const Problem& problem, const ActionPlan& plan, const State& s0) {
		return print_plan_execution(problem, transform(problem, plan), s0);
	}
	
	static void print_plan_execution(const Problem& problem, const std::vector<LiftedActionID>& plan, const State& s0) {
		return print_plan_execution(problem, transform(problem, plan), s0);
	}
};


} // namespaces
