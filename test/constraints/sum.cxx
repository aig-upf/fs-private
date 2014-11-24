

#include <string.h>
#include <boost/concept_check.hpp>
#include <gtest/gtest.h>

#include <core_types.hxx>
#include <constraints/sum.hxx>
#include <fixtures/constraint_fixture.hxx>

using namespace aptk::core;


class SumConstraintTest : public aptk::test::constraints::ConstraintFixture {
 protected:

  virtual void SetUp() {
	
	  vars.push_back({2, 4, 6, 8});
	  domains.push_back(buildDomainSet({
			{2, {1, 2, 3}},
			{4, {1, 2, 3}},
			{6, {1, 2, 3}},
			{8, {1, 2, 3}}
		}));
	  pruned.push_back({{1}, {1}, {1}, {3}});
	  codes.push_back(Constraint::Output::Pruned);
	  
	  
	  
	  vars.push_back({2, 4, 6});
	  domains.push_back(buildDomainSet({
			{2, {0, 7, 10}},
			{4, {1, 8, 11}},
			{6, {1, 6, 18}},
		}));
	  pruned.push_back({{0, 7, 10}, {1, 8, 11}, {1, 6, 18}});
	  codes.push_back(Constraint::Output::Unpruned);
	  
  }

  // A helper tester
	void testPropagatorWithCase(unsigned i) {
		SumConstraint ctr(vars[i]);
		EXPECT_EQ(codes[i],  ctr.enforce_consistency(domains[i]));
		EXPECT_EQ(pruned[i], ctr.getCurrentDomains());
	}

	std::vector<DomainSet> domains;
	std::vector<std::vector<std::vector<int>>> pruned;
	std::vector<Constraint::Output> codes;
	std::vector<VariableIdxVector> vars;
};


TEST_F(SumConstraintTest, PrunableDomain) {
 	testPropagatorWithCase(0);
}
 
TEST_F(SumConstraintTest, ValidDomains) {
  	testPropagatorWithCase(1);
}
