
#pragma once

#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace fs0 {

// using type_id   = uint16_t;
enum class type_id : uint16_t {
	bool_t,
	ushort_t,
	short_t,
	uint_t,
	int_t,
	ufloat_t,
	float_t,
	
	set_t,
	interval_t
	// ...
};

/**
 * First 8 bits for type, rest for value.
 */
/*
class object_id {
public:
    using data_t = uint64_t;
    
    template <typename T>
    object_id(type_id t, T value) 
        : _data(to_data(t, value))
    {}
    
    
    template <typename T>
    static data_t to_data(type_id t, T value) {
        static_assert(sizeof(T) <= 7, "Unsupported object_id value type");
        return ((data_t) t << 56) | value; 
    }
    
private:
    data_t _data;
};

type_id type(object_id o) {
	return (type_id) ((o & 0xFFFF000000000000) >> 48);
}
*/

class object_id {
public:
    using value_t = uint32_t;
    
    template <typename T>
    object_id(type_id t, T val) 
        : type(t), value(val)
    { static_assert(sizeof(T) <= sizeof(value_t), "Unsupported object_id value type"); }
    
    
    const type_id type;
	const value_t value;
};
static_assert(sizeof(object_id) == 8, "object_id should have overall size equal to 64 bits");

uint64_t hash(const object_id& o);



class ObjectTable {
public:
	
};

class set_t {
public:
    
};


class type_mismatch_error : public std::runtime_error {
public:
	type_mismatch_error() : std::runtime_error("Type Mismatch Error") {}
};

class value_mismatch_error : public std::runtime_error {
public:
	value_mismatch_error() : std::runtime_error("Value Mismatch Error") {}
};


inline type_id
o_type(const object_id& o) { return o.type; }

template <typename T>
T value(const object_id& o, const ObjectTable& itp);

template <>
bool value(const object_id& o, const ObjectTable& itp);

template <>
uint32_t value(const object_id& o, const ObjectTable& itp);

template <>
int32_t value(const object_id& o, const ObjectTable& itp);


template <typename T>
object_id make_obj(const T& value);

template <>
object_id make_obj(const bool& value);

template <>
object_id make_obj(const uint32_t& value);

template <>
object_id make_obj(const int32_t& value);


/*
template <>
set_t value(const object_id& o, const ObjectTable& itp) {
	if (o.type != type_id::set_t) throw type_mismatch_error();
	// Even additional checks with assert: the value must be either 0 or 1!
    return set_t();
}
*/



} // namespaces
