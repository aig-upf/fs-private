
#pragma once

#include <fs/core/fs_types.hxx>

namespace fs0 {

class State;
class Problem;
class LiftedActionID;
class SchematicActionID;
class GroundAction;

class Checker {
public:
	//! Transform different plan formats into a vector of ground actions.
	static std::vector<const GroundAction*> transform(const Problem& problem, const std::vector<LiftedActionID>& plan);
    static std::vector<const GroundAction*> transform(const Problem& problem, const std::vector<SchematicActionID>& plan);
    static std::vector<const GroundAction*> transform(const Problem& problem, const ActionPlan& plan);

	static bool check_correctness(const Problem& problem, const std::vector<const GroundAction*>& plan, const State& s0);

	static void print_plan_execution(const Problem& problem, const std::vector<const GroundAction*>& plan, const State& s0);
};


} // namespaces
