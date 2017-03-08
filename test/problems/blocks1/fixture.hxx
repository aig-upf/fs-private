
#pragma once

#include "fixtures/base_fixture.hxx"
#include "generator.hxx"
#include <core_state.hxx>

using namespace aptk::core;

namespace aptk { namespace test { namespace problems { namespace blocks1 {

class Blocks1Fixture : public testing::Test {

protected:
	virtual void SetUp() {
		problem_ = std::shared_ptr<Problem>(new Problem());
		generate(*problem_);
		fwdProblem_ = std::shared_ptr<FwdSearchProblem>(new FwdSearchProblem(*problem_));
	}
	
	virtual void TearDown() {
	}
	
	virtual CoreState getInvertedGoalState() {
		return generateInvertedGoalState(problem_->get_symbol_table());
	}
	
	virtual CoreState getIncGoalState() {
		return generateIncGoalState(problem_->get_symbol_table());
	}	
	
	virtual CoreState getGoalState() {
		return generateGoalState(problem_->get_symbol_table());
	}
  
	std::shared_ptr<FwdSearchProblem> fwdProblem_;
	std::shared_ptr<Problem> problem_;
};

} } } } // namespaces
