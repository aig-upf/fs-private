
#pragma once

#include <typeinfo>
#include <functional>

namespace fs0 {

	typedef  std::reference_wrapper<const std::type_info> TypeInfoRef;

	struct TypeInfoHasher {
		std::size_t operator()(TypeInfoRef code) const { return code.get().hash_code(); }
	};

	struct TypeInfoEqualsTo {
		bool operator()( TypeInfoRef lhs, TypeInfoRef rhs ) const { return lhs.get() == rhs.get(); }
	};

} // namespaces

