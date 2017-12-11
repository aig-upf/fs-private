
#include <fs/core/utils/printers/binding.hxx>
#include <fs/core/utils/binding.hxx>
#include <fs/core/problem_info.hxx>

namespace fs0 { namespace print {


std::ostream& binding::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (unsigned i = 0; i < _binding.size(); ++i) {
		if (!_binding.binds(i)) continue;
		os << info.object_name(_binding.value(i));
		if (i < _binding.size() - 1) os << ", ";
	}
	return os;
}

std::ostream& signature::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (unsigned i = 0; i < _parameter_names.size(); ++i) {
		os << _parameter_names[i] << ": " << info.getTypename(_signature[i]);
		if (i < _parameter_names.size() - 1) os << ", ";
	}
	return os;
}

std::ostream& raw_signature::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (unsigned i = 0; i < _signature.size(); ++i) {
		os << info.getTypename(_signature[i]);
		if (i < _signature.size() - 1) os << ", ";
	}
	return os;
}

partial_binding::partial_binding(const std::vector<std::string>& parameter_names, const Binding& binding, const Signature& signature) : _parameter_names(parameter_names), _binding(binding), _signature(signature) {
	assert(parameter_names.size() == binding.size());
}

std::ostream& partial_binding::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (unsigned i = 0; i < _parameter_names.size(); ++i) {
		if (_binding.binds(i)) {
			os << info.object_name(_binding.value(i));
		} else {
			os << _parameter_names[i];
		}
		if (i < _parameter_names.size() - 1) os << ", ";
	}
	return os;
}


strips_partial_binding::strips_partial_binding(const std::vector<std::string>& parameter_names, const Binding& binding, const Signature& signature) : _parameter_names(parameter_names), _binding(binding), _signature(signature) {
	assert(parameter_names.size() == binding.size());
}

std::ostream& strips_partial_binding::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (unsigned i = 0; i < _parameter_names.size(); ++i) {
		if (_binding.binds(i)) {
			os << info.object_name(_binding.value(i));
		} else {
			os << _parameter_names[i];
		}
		if (i < _parameter_names.size() - 1) os << " ";
	}
	return os;
}



} } // namespaces
