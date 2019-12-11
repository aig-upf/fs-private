
#include <cstring>
#include <vector>
#include <iostream>
#include <climits>
#include <unordered_map>

#include <fs/core/base.hxx>
#include <fs/core/utils/utils.hxx>


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

type_id from_string(const std::string& t) {
	static std::unordered_map<std::string, type_id> mapping{
		{ "object_t", type_id::object_t },
		{ "bool_t", type_id::bool_t },
		{ "int_t", type_id::int_t },
		{ "float_t", type_id::float_t },
		{ "set_t", type_id::set_t },
		{ "interval_t", type_id::interval_t }
	};

	auto it = mapping.find( t );
	if (it == mapping.end()) throw std::runtime_error( "Error resolving underlying data type for identifier '" + t + "'" );
	return it->second;
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
	if (t != type_id::int_t && t != type_id::object_t && t != type_id::bool_t) {
		throw type_mismatch_error();
	}
	return (int32_t) o.value();
}


template <>
float value(const object_id& o) {
	if (o.type() != type_id::float_t) throw type_mismatch_error();
	object_id::value_t val = o.value();
	float tmp;
	Utils::type_punning_without_aliasing(val, tmp);
	return tmp;
}


template <typename T>
std::vector<T> values(const std::vector<object_id>& os, const ObjectTable& itp) {
	std::vector<T> result;
	result.reserve(os.size());
	for (const auto& o:os) result.emplace_back(value<T>(o, itp));
	return result;
}

template std::vector<int> values<int>(const std::vector<object_id>& os, const ObjectTable& itp); // explicit instantiation.



template <typename T>
object_id make_object(const T& value) {
	// Provoke a compile-time error if ever instantiated with an actual type T
	static_assert(sizeof(T) == 0, "fs0::make_object() needs to be defined for type");
	return object_id::INVALID; // prevents a g++ warning
}

template <>
object_id make_object(const object_id& value) {
	throw std::runtime_error("Suspicious use of make_object(): from object_id to object_id");
}


template <>
object_id make_object(const bool& value) {
	return object_id(type_id::bool_t, value);
}


template <>
object_id make_object(const int32_t& value) {
	return object_id(type_id::int_t, value);
}

template <>
object_id make_object(const float& value) {
	int32_t tmp;
	Utils::type_punning_without_aliasing(value, tmp);
	return object_id(type_id::float_t, tmp);
}

template <>
object_id make_object(const double& value) {
    throw std::runtime_error("Suspicious use of make_object(): from double to object_id");
}

std::ostream& object_id::print(std::ostream& os) const {
	if (_type == type_id::invalid_t) return os << "INV!";
	
	// For some types, we will want to properly unpack the value to print it
	if (_type == type_id::int_t) os << fs0::value<int>(*this);
	else if (_type == type_id::float_t) os << fs0::value<float>(*this);
	else os << _value;
	
	return os << _type;
}

} // namespaces
