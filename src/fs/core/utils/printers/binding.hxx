
#pragma once

#include <vector>
#include <ostream>
#include <fs/core/fs_types.hxx>

namespace fs0 { class Binding; }

namespace fs0 { namespace print {

class binding {
	protected:
		const Binding& _binding;
		const Signature& _signature;
	public:
		binding(const Binding& binding, const Signature& signature) : _binding(binding), _signature(signature) {
			//assert(binding.size() == signature.size());
		}
		
		friend std::ostream& operator<<(std::ostream &os, const binding& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

class partial_binding {
	protected:
		const std::vector<std::string>& _parameter_names;
		const Binding& _binding;
		const Signature& _signature;
		
	public:
		partial_binding(const std::vector<std::string>& parameter_names, const Binding& binding, const Signature& signature);
		
		friend std::ostream& operator<<(std::ostream &os, const partial_binding& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

class signature {
	protected:
		const std::vector<std::string>& _parameter_names;
		const Signature& _signature;
	public:
		signature(const std::vector<std::string>& parameter_names, const Signature& sign) : _parameter_names(parameter_names), _signature(sign) {
			assert(parameter_names.size() == sign.size());
		}
		
		friend std::ostream& operator<<(std::ostream &os, const signature& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Print only signature types, without parameter names whatsoever
class raw_signature {
	protected:
		const Signature& _signature;
	public:
		raw_signature(const Signature& sign) : _signature(sign) {}
		
		friend std::ostream& operator<<(std::ostream &os, const raw_signature& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
