
#pragma once

#include <vector>
#include <ostream>
#include <fs0_types.hxx>

namespace fs0 { namespace print {

class binding {
	protected:
		const std::vector<int>& _binding;
		const Signature& _signature;
	public:
		binding(const std::vector<int>& binding, const Signature& signature) : _binding(binding), _signature(signature) {
			assert(binding.size() == signature.size());
		}
		
		friend std::ostream& operator<<(std::ostream &os, const binding& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

class signature {
	protected:
		const std::vector<std::string>& _parameter_names;
		const Signature& _signature;
	public:
		signature(const std::vector<std::string>& parameter_names, const Signature& signature) : _parameter_names(parameter_names), _signature(signature) {
			assert(parameter_names.size() == signature.size());
		}
		
		friend std::ostream& operator<<(std::ostream &os, const signature& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};



} } // namespaces
