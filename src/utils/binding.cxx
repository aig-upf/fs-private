
#include <utils/binding.hxx>
#include <iostream>

namespace fs0 {

const Binding Binding::EMPTY_BINDING = Binding();

std::ostream& Binding::
print(std::ostream& os) const {
	os << "{";
	for (unsigned i = 0; i < _values.size(); ++i) {
		if (!binds(i)) continue;
		os << i << ": " << value(i) << ", ";
	}
	os << "}";
	return os;
}

} // namespaces
