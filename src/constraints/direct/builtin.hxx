
#pragma once

#include <constraints/direct/constraint.hxx>
#include <constraints/direct/effect.hxx>


namespace fs0 {

class ProblemInfo;

// X < Y
class LTConstraint : public BinaryDirectConstraint
{
public:
	LTConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~LTConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }
	
	FilteringOutput filter(unsigned variable);
	
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X <= Y
class LEQConstraint : public BinaryDirectConstraint
{
public:
	LEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~LEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	FilteringOutput filter(unsigned variable);
	
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};


// X = Y
class EQConstraint : public BinaryDirectConstraint
{
public:
	EQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~EQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 == o2; }
	
	FilteringOutput filter(unsigned variable);
	
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X != Y
class NEQConstraint : public BinaryDirectConstraint
{
public:
	NEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~NEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }
	
	FilteringOutput filter(unsigned variable);
	
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X = c
class EQXConstraint : public UnaryDirectConstraint
{
public:
	EQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~EQXConstraint() {};

	bool isSatisfied(ObjectIdx o) const { return o == _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const;
	
	// No compilation
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X != c
class NEQXConstraint : public UnaryDirectConstraint
{
public:
	NEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~NEQXConstraint() {};

	bool isSatisfied(ObjectIdx o) const { return o != _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const;
	
	// No compilation
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

// X := c, where X os the affected variable, and c is the only parameter
class ValueAssignmentEffect : public ZeroaryDirectEffect {
public:
	ValueAssignmentEffect(VariableIdx affected, ObjectIdx value);
	
	virtual Atom apply() const;
	
	virtual std::ostream& print(std::ostream& os) const;
};

// X := Y
class VariableAssignmentEffect : public UnaryDirectEffect
{
public:
	VariableAssignmentEffect(VariableIdx relevant, VariableIdx affected);
	
	Atom apply(ObjectIdx v1) const;
	
	virtual std::ostream& print(std::ostream& os) const;
};

// X = Y + c (with X and Y possibly being the same variable)
class AdditiveUnaryEffect : public UnaryDirectEffect {
public:
	AdditiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value);

	Atom apply(ObjectIdx v1) const;

	virtual std::ostream& print(std::ostream& os) const;
};



} // namespaces

