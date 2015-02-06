
#pragma once

#include <constraints/scoped_constraint.hxx>
#include <problem_info.hxx>


namespace fs0 {

// X >= Y
class GEQConstraint : public BinaryParametrizedScopedConstraint
{
public:
	GEQConstraint(const VariableIdxVector& scope) :
		BinaryParametrizedScopedConstraint(scope, {}) {}
	virtual ~GEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
		return o1 >= o2;
	}
};

// X <= Y
class LEQConstraint : public BinaryParametrizedScopedConstraint
{
public:
	LEQConstraint(const VariableIdxVector& scope) :
		BinaryParametrizedScopedConstraint(scope, {}) {}
	virtual ~LEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	Output filter(unsigned variable);
};


} // namespaces

