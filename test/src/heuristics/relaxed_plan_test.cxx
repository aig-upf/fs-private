

#include <string.h>
#include <boost/concept_check.hpp>
#include <gtest/gtest.h>

#include <heuristics/relaxed_plan.hxx>
#include <core_fwd_search_prob.hxx>

#include "problems/simple1/fixture.hxx"

using namespace aptk::core;
using namespace aptk::test::problems::simple1;

typedef RelaxedPlanHeuristic<FwdSearchProblem> RelaxedHeuristic;

class RPHeuristicTest : public SimpleProblem1Fixture {};

// Test the heuristic value of a very basic state with two independent variables achieved by the same action.
TEST_F(RPHeuristicTest, InitialStateHeuristic) {
	float h = 0.0;
	RelaxedHeuristic heuristic(*fwdProblem_);
	heuristic.eval( *(fwdProblem_->init()), h );
	EXPECT_EQ(1.0, h);
}

TEST_F(RPHeuristicTest, State1Heuristic) {
	float h = 0.0;
	RelaxedHeuristic heuristic(*fwdProblem_);
	heuristic.eval( getState1(), h );
	EXPECT_EQ(1.0, h);
}
