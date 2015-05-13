
#include <atoms.hxx>
#include <problem.hxx>

namespace fs0 {

Atom::Atom(const VariableIdx variable, const ObjectIdx value) :
	_variable(variable), _value(value)
{}
	
std::ostream& Atom::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << "[" << problemInfo.getVariableName(_variable) << "=" << problemInfo.getObjectName(_variable, _value) << "]";
	return os;
}

} // namespaces
