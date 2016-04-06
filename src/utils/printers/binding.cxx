
#include <utils/printers/binding.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace print {


std::ostream& binding::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	for (unsigned i = 0; i < _binding.size(); ++i) {
		if (!_binding.binds(i)) continue;
		os << info.deduceObjectName(_binding.value(i), _signature[i]);
		if (i < _binding.size() - 1) os << ", ";
	}
	return os;
}

std::ostream& signature::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	for (unsigned i = 0; i < _parameter_names.size(); ++i) {
		os << _parameter_names[i] << ": " << info.getTypename(_signature[i]);
		if (i < _parameter_names.size() - 1) os << ", ";
	}
	return os;
}

std::ostream& partial_binding::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	for (unsigned i = 0; i < _parameter_names.size(); ++i) {
		if (_binding.binds(i)) {
			os << info.deduceObjectName(_binding.value(i), _signature[i]);
		} else {
			os << _parameter_names[i];
		}
		if (i < _parameter_names.size() - 1) os << ", ";
	}
	return os;
}




} } // namespaces
