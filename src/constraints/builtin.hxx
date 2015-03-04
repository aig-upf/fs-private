
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


// X = Y
class EQConstraint : public BinaryParametrizedScopedConstraint
{
public:
	EQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~EQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 == o2; }
	
	Output filter(unsigned variable);
	
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X != Y
class NEQConstraint : public BinaryParametrizedScopedConstraint
{
public:
	NEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~NEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }
	
	Output filter(unsigned variable);
	
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X != c
class NEQXConstraint : public UnaryParametrizedScopedConstraint
{
public:
	NEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~NEQXConstraint() {};

	bool isSatisfied(ObjectIdx o) const { return o != _binding[0]; }
	
	Output filter(const DomainMap& domains) const;
	
	// No compilation
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

} // namespaces

