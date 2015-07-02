
#include <constraints/builtin_effects.hxx>

namespace fs0 {

ValueAssignmentEffect::ValueAssignmentEffect(const VariableIdx affected, const ObjectIdx value) 
	: ZeroaryScopedEffect({}, affected, {value})
{}

Atom ValueAssignmentEffect::apply() const {
	assert(applicable());
	return Atom(_affected, _binding[0]);
}

std::ostream& ValueAssignmentEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	os << info.getVariableName(_affected) << " := " << info.getObjectName(_affected, _binding[0]);
	return os;
}


AdditiveUnaryEffect::AdditiveUnaryEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters)
	: UnaryScopedEffect(scope, affected, parameters)
{
	assert(parameters.size()==1);
}
		
		
Atom AdditiveUnaryEffect::apply(ObjectIdx v1) const {
	assert(applicable(v1));
	return Atom(_affected, v1 + _binding[0]);
}

std::ostream& AdditiveUnaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " + " << _binding[0];
	return os;
}

} // namespaces
