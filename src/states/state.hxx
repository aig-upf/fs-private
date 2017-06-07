
#pragma once

#include <vector>

#include <base.hxx>
// #include <state.hxx>

namespace fs0 {
	
class GenericState {
protected:
	std::vector<object_id> _values;

	std::size_t _hash;
	
	
// 	std::size_t compute_hash() const { return hash_value(_values[0]); }
public:
	
};

template <typename T>
const T evaluate(const GenericState& state, unsigned variable);

template <>
const int evaluate(const GenericState& state, unsigned variable) {
    //return state.getIntValue(variable);
    return 0;
}

} // namespaces
