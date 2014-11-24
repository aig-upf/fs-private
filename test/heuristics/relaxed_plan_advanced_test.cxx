

#include <string.h>
#include <boost/concept_check.hpp>
#include <gtest/gtest.h>

#include <heuristics/relaxed_plan.hxx>
#include <core_fwd_search_prob.hxx>

#include "problems/blocks1/fixture.hxx"

using namespace aptk::core;
using namespace aptk::test::problems::blocks1;

typedef RelaxedPlanHeuristic<FwdSearchProblem> RelaxedHeuristic;

// Test the heuristic values on a simple blocksworld instance.
class RPHeuristicAdvancedTest : public Blocks1Fixture {};


TEST_F(RPHeuristicAdvancedTest, InitialStateHeuristic) {
	float h = 0.0;
	RelaxedHeuristic heuristic(*fwdProblem_);
	heuristic.eval( *(fwdProblem_->init()), h );
	EXPECT_EQ(6.0, h);
}


TEST_F(RPHeuristicAdvancedTest, InvertedGoalHeuristic) {
	float h = 0.0;
	RelaxedHeuristic heuristic(*fwdProblem_);
	heuristic.eval( getInvertedGoalState(), h );
	// Note that on the computation of the delete-heuristic we don't need to execute put-down(a),
	// as the goal is actually underspecified and does not explicitly require ontable(a).
	EXPECT_EQ(7.0, h);
}

TEST_F(RPHeuristicAdvancedTest, IncrementalGoalHeuristic) {
	float h = 0.0;
	RelaxedHeuristic heuristic(*fwdProblem_);
	heuristic.eval( getIncGoalState(), h );
	EXPECT_EQ(5.0, h);
}

TEST_F(RPHeuristicAdvancedTest, GoalStateHeuristic) {
	float h = 0.0;
	RelaxedHeuristic heuristic(*fwdProblem_);
	heuristic.eval( getGoalState(), h );
	EXPECT_EQ(0.0, h);
}