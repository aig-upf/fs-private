
#include <constraints/builtin_effects.hxx>

namespace fs0 {

ValueAssignmentEffect::ValueAssignmentEffect(const VariableIdx variable, const ObjectIdx value) 
	: ZeroaryScopedEffect({}, {variable}, {value})
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



} // namespaces
