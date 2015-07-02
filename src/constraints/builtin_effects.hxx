
#pragma once

#include <constraints/scoped_effect.hxx>
#include <problem_info.hxx>
#include <problem.hxx>


namespace fs0 {

// X := value
class ValueAssignmentEffect : public ZeroaryScopedEffect
{
public:
	ValueAssignmentEffect(const VariableIdx variable, const ObjectIdx value);
	virtual ~ValueAssignmentEffect() {};
	
	virtual Atom apply() const;
	
	virtual std::ostream& print(std::ostream& os) const;
};


} // namespaces

