
#include <atom.hxx>
#include <problem.hxx>

namespace fs0 {

std::ostream& Atom::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getInfo();
	os << "[" << problemInfo.getVariableName(_variable) << "=" << problemInfo.getObjectName(_variable, _value) << "]";
	return os;
}

} // namespaces
