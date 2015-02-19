
#pragma once

#include <constraints/scoped_constraint.hxx>
#include <problem_info.hxx>


namespace fs0 {

// X < Y
class LTConstraint : public BinaryParametrizedScopedConstraint
{
public:
	LTConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~LTConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }
	
	Output filter(unsigned variable);
	
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X <= Y
class LEQConstraint : public BinaryParametrizedScopedConstraint
{
public:
	LEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~LEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	Output filter(unsigned variable);
	
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};


} // namespaces

