
#pragma once

#include <vector>
#include <ostream>

namespace fs0 { namespace print {

	
template <typename T>
class Vector {
	protected:
		const std::vector<T>& _object;
	public:
		Vector(const std::vector<T>& object) : _object(object) {}
		
		friend std::ostream& operator<<(std::ostream &os, const Vector& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const {
			os << "{";
			for (unsigned i = 0; i < _object.size(); ++i) {
				os << _object[i];
				if (i < _object.size() - 1) os << ", ";
			}
			os << "}";
			return os;
		}
};

//! A helper to allow for automatic template argument deduction
template <typename T>
const Vector<T> vector(const std::vector<T>& object) { return Vector<T>(object); }
}

} // namespaces
