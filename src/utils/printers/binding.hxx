
#pragma once

#include <vector>
#include <ostream>
#include <fs0_types.hxx>

namespace fs0 { namespace print {

	
class binding {
	protected:
		const std::vector<int>& _binding;
		const std::vector<TypeIdx>& _signature;
	public:
		binding(const std::vector<int>& binding, const std::vector<TypeIdx>& signature) : _binding(binding), _signature(signature) {
			assert(binding.size() == signature.size());
		}
		
		friend std::ostream& operator<<(std::ostream &os, const binding& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
