
#pragma once

#include <constraints/scoped_effect.hxx>
#include <problem_info.hxx>
#include <problem.hxx>


namespace fs0 {

// X := c
class ValueAssignmentEffect : public ZeroaryScopedEffect
{
public:
	ValueAssignmentEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters);
	
	virtual Atom apply() const;
	
	virtual std::ostream& print(std::ostream& os) const;
};

// X := Y
class VariableAssignmentEffect : public UnaryScopedEffect
{
public:
	VariableAssignmentEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters);
	
	Atom apply(ObjectIdx v1) const;
	
	virtual std::ostream& print(std::ostream& os) const;
};

// X = Y + c (with X and Y possibly being the same variable.
class AdditiveUnaryEffect : public UnaryScopedEffect {
public:
	AdditiveUnaryEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters);

	Atom apply(ObjectIdx v1) const;

	virtual std::ostream& print(std::ostream& os) const;
};


} // namespaces

