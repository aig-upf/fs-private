
#pragma once

#include <fs0_types.hxx>

namespace fs0 { namespace language { namespace fstrips {

//! A number of helper methods
class FStripsUtils {
public:
	template <typename T>
	static std::vector<T> clone(const std::vector<T>& elements) {
		std::vector<T> cloned;
			for (auto element:elements) {
			cloned.push_back(element->clone());
		}
		return cloned;
	}
};

} } } // namespaces
