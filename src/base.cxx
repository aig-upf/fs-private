
#include <cstring>

#include <base.hxx>


namespace fs0 {
    
void dummy_test() {
	ObjectTable ot;
	object_id o(type_id::int_t, 5);
	value<int>(o, ot);
	
	
//  	value<char>(o, ot);
}


uint64_t hash(const object_id& o) {
	static_assert(sizeof(o) == sizeof(uint64_t), "Size mismatch");
	uint64_t result;
	std::memcpy(&result, &o, sizeof result);
	return result;
}

template <typename T>
T value(const object_id& o, const ObjectTable& itp) {
	// Provoke a compile-time error if ever instantiated with an actual type T
	static_assert(sizeof(T) == 0, "fs0::value() needs to be defined for type");
};



template <>
bool value(const object_id& o, const ObjectTable& itp) {
	if (o.type != type_id::bool_t) throw type_mismatch_error();
	assert(o.value == 0 || o.value == 1);
	return (bool) o.value;
}

template <>
int32_t value(const object_id& o, const ObjectTable& itp) {
	if (o.type != type_id::int_t) throw type_mismatch_error();
	return (int32_t) o.value;
}

template <>
uint32_t value(const object_id& o, const ObjectTable& itp) {
	if (o.type != type_id::uint_t) throw type_mismatch_error();
	return (uint32_t) o.value;
}


template <typename T>
object_id make_obj(const T& value) {
	// Provoke a compile-time error if ever instantiated with an actual type T
	static_assert(sizeof(T) == 0, "fs0::make_obj() needs to be defined for type");
}

template <>
object_id make_obj(const bool& value) {
	return object_id(type_id::bool_t, value);
}

template <>
object_id make_obj(const uint32_t& value) {
	return object_id(type_id::uint_t, value);
}

template <>
object_id make_obj(const int32_t& value) {
	return object_id(type_id::int_t, value);
}


} // namespaces
