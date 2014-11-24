

#include <string.h>
#include <boost/concept_check.hpp>
#include <gtest/gtest.h>

#include <fixtures/constraint_fixture.hxx>

using namespace aptk::core;

class AlldiffConstraintFixture : public aptk::test::constraints::ConstraintFixture {

protected:
	virtual void SetUp() {
		VariableIdxVector three_vars{2, 4, 6};
		arity3_constraint = std::make_shared<AlldiffConstraint>(three_vars);
		
		VariableIdxVector two_vars{1, 2};
		arity2_constraint = std::make_shared<AlldiffConstraint>(two_vars);
		
		// D_1 = D_2 = D_3 = {1, 2}
		failure_domains_ = buildDomainSet({{2, {1,2}}, {4, {1,2}},{6, {1,2}}});
		
		
		prunable_domains_min_ = buildDomainSet({
			{2, {1,2}},
			{4, {1,2}},
			{6, {2,3}}
		});
		
		unordered_prunable_domains_min_ = buildDomainSet({
			{1, {1,2}},
			{3, {2,3}},
			{8, {1,2}}
		});
		
		
		// The 3 of the 3rd domain should be pruned?
		prunable_domains_max_ = buildDomainSet({
			{2, {3,4}},
			{4, {3,4}},
			{6, {2,3}}
		});
		
		valid_domains_ = buildDomainSet({
			{2, {1,2}}, {4, {1,2}}
		});
	}
  
	std::shared_ptr<AlldiffConstraint> arity3_constraint;
	std::shared_ptr<AlldiffConstraint> arity2_constraint;
	
	DomainSet failure_domains_;
	DomainSet prunable_domains_min_;
	DomainSet unordered_prunable_domains_min_;
	DomainSet prunable_domains_max_;
	DomainSet valid_domains_;
};

class AlldiffTest : public AlldiffConstraintFixture {};

TEST_F(AlldiffTest, FailureDomains) {
 	EXPECT_EQ(Constraint::Output::Failure, arity3_constraint->enforce_consistency(failure_domains_));
}
 
TEST_F(AlldiffTest, ValidDomains) {
 	EXPECT_EQ(Constraint::Output::Unpruned, arity2_constraint->enforce_consistency(valid_domains_));
}

TEST_F(AlldiffTest, PrunableDomainsMin) {
	EXPECT_EQ(Constraint::Output::Pruned, arity3_constraint->enforce_consistency(prunable_domains_min_));
	
	std::vector<std::vector<int>> expected{{1,2}, {1,2}, {3}};
	EXPECT_EQ(expected, arity3_constraint->getCurrentDomains());
}

TEST_F(AlldiffTest, PrunableDomainsMinUnordered) {
	EXPECT_EQ(Constraint::Output::Pruned, arity3_constraint->enforce_consistency(unordered_prunable_domains_min_));
	
	std::vector<std::vector<int>> expected{{1,2}, {3}, {1,2}};
	EXPECT_EQ(expected, arity3_constraint->getCurrentDomains());
}

TEST_F(AlldiffTest, PrunableDomainsMax) {
	EXPECT_EQ(Constraint::Output::Pruned, arity3_constraint->enforce_consistency(prunable_domains_max_));
}