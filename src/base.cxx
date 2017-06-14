
#include <cstring>
#include <vector>
#include <iostream>
#include <type_traits>
#include <climits>

#include <base.hxx>


namespace fs0 {
    

const object_id object_id::INVALID = object_id();
const object_id object_id::FALSE = object_id(type_id::bool_t, false);
const object_id object_id::TRUE = object_id(type_id::bool_t, true);

const ObjectTable ObjectTable::EMPTY_TABLE = ObjectTable();

std::string to_string(const type_id& t) {
	if (t == type_id::invalid_t)     return "!";
	if (t == type_id::object_t)      return "o";
	if (t == type_id::bool_t)        return "b";
// 	if (t == type_id::ushort_t)      return "US";
// 	if (t == type_id::short_t)       return "S";
// 	if (t == type_id::uint_t)        return "UI";
	if (t == type_id::int_t)         return "i";
// 	if (t == type_id::ufloat_t)      return "UF";
	if (t == type_id::float_t)       return "f";
	if (t == type_id::set_t)         return "SET";
	if (t == type_id::interval_t)    return "RNG";
	return "?";
}

std::ostream& operator<<(std::ostream &os, const type_id& t) { return os << to_string(t); }



std::size_t hash_value(const object_id& o) {
	static_assert(sizeof(object_id) <= sizeof(std::size_t), "Size mismatch");
	
	std::size_t ctype = (std::size_t) static_cast<std::underlying_type_t<type_id>>(o.type());
	return (ctype << (CHAR_BIT * sizeof(object_id::value_t))) | o.value();
}


template <typename T>
T value(const object_id& o, const ObjectTable& itp) {
	// Provoke a compile-time error if ever instantiated with an actual type T
	static_assert(sizeof(T) == 0, "fs0::value() needs to be defined for type");
}


template <>
bool value(const object_id& o) {
	if (o.type() != type_id::bool_t) throw type_mismatch_error();
	assert(o.value() == 0 || o.value() == 1);
	return (bool) o.value();
}


template <>
int32_t value(const object_id& o) {
	const auto& t = o.type();
	if (t != type_id::int_t && t != type_id::object_t) {
		throw type_mismatch_error();
	}
	return (int32_t) o.value();
}

template <typename T>
std::vector<T> values(const std::vector<object_id>& os, const ObjectTable& itp) {
	std::vector<T> result;
	for (const auto& o:os) result.push_back(value<T>(o, itp));
	return result;
}

template std::vector<int> values<int>(const std::vector<object_id>& os, const ObjectTable& itp); // explicit instantiation.



template <typename T>
object_id make_obj(const T& value) {
	// Provoke a compile-time error if ever instantiated with an actual type T
	static_assert(sizeof(T) == 0, "fs0::make_obj() needs to be defined for type");
	return object_id::INVALID; // prevents a g++ warning
}

template <>
object_id make_obj(const bool& value) {
	return object_id(type_id::bool_t, value);
}


template <>
object_id make_obj(const int32_t& value) {
	return object_id(type_id::int_t, value);
}

std::ostream& object_id::print(std::ostream& os) const {
	if (_type == type_id::invalid_t) return os << "INV!";
	return os << _value << _type;
}

} // namespaces
