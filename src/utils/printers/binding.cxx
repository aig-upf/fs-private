
#include <utils/printers/binding.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace print {


std::ostream& binding::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	os << "{";
	for (unsigned i = 0; i < _binding.size(); ++i) {
		os << info.deduceObjectName(_binding[i], _signature[i]);
		if (i < _binding.size() - 1) os << ", ";
	}
	os << "}";
	return os;
}



} } // namespaces
