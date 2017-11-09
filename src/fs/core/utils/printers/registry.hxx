
#pragma once

#include <vector>
#include <ostream>
#include <fs/core/fs_types.hxx>

namespace fs0 {

class LogicalComponentRegistry; namespace print {

class logical_registry {
	protected:
		const LogicalComponentRegistry& _registry;
	public:
		logical_registry(const LogicalComponentRegistry& registry) : _registry(registry) {}
		
		friend std::ostream& operator<<(std::ostream &os, const logical_registry& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
