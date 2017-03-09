

#include <string.h>
#include <boost/concept_check.hpp>
#include <gtest/gtest.h>

#include <heuristics/relaxed_plan.hxx>
#include <core_fwd_search_prob.hxx>

#include "problems/simple1/fixture.hxx"

using namespace aptk::core;
using namespace aptk::test::problems::simple1;

class Basic1Test : public SimpleProblem1Fixture {};

// Test the generation of a simple problem.
TEST_F(Basic1Test, ProblemGeneration) {
	EXPECT_EQ(problem_->get_num_types(), 1);
	EXPECT_EQ(problem_->get_num_objects(), 2);
	EXPECT_EQ(problem_->get_num_actions(), 1);
	
	EXPECT_EQ(problem_->getNumBoundActions(), 1);
	EXPECT_EQ(problem_->get_type_id("object"), 0);
}


// Test the goal checking
TEST_F(Basic1Test, GoalChecking) {
	EXPECT_TRUE(problem_->is_goal(getGoalState()));
}