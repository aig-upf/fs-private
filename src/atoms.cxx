
#include <atoms.hxx>
#include <problem.hxx>

namespace fs0 {

Fact::Fact(const VariableIdx variable, const ObjectIdx value) :
	_variable(variable),
	_value(value)
{};
	
std::ostream& Fact::print(std::ostream& os) const {
	auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << "[" << problemInfo->getVariableName(_variable) << getSign() << problemInfo->getObjectName(_variable, _value) << "]";
	return os;
}

} // namespaces
