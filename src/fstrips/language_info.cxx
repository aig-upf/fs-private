
#include <unordered_map>

#include <lapkt/tools/logging.hxx>

#include <fstrips/language_info.hxx>
#include <utils/loader.hxx>


namespace fs0 { namespace fstrips {

// TODO - Exceptions could be improved to receive objects and compose the error message themselves.
class unregistered_symbol : public std::runtime_error {
public: unregistered_symbol(const std::string& msg) : std::runtime_error(msg) {}
};

class unregistered_type : public std::runtime_error {
public: unregistered_type(const std::string& msg) : std::runtime_error(msg) {}
};

class unregistered_object : public std::runtime_error {
public: unregistered_object(const std::string& msg) : std::runtime_error(msg) {}
};

class range_type_mismatch : public std::runtime_error {
public: range_type_mismatch(const type_id& t1, const type_id& t2) : 
	std::runtime_error("Mismatched object types in range: \"" + to_string(t1) + "\" vs. \"" + to_string(t2) + "\"") {}
};

class invalid_range : public std::runtime_error {
public: invalid_range(const std::string& msg) : std::runtime_error(msg) {}
};
	
// TODO - REMOVE THIS
std::unique_ptr<LanguageInfo> LanguageInfo::_instance = nullptr;

	
class LanguageInfo::Implementation {
private:
	friend class LanguageInfo;
	
	Implementation() {

		// The first object Ids are reserved for Booleans
		TypeIdx bool_t = add_fstype("bool");
		object_id o_false = add_object("*false*", bool_t);
		object_id o_true = add_object("*true*", bool_t);
		
// 		LPT_INFO("cout", "o_false: " << o_false);
// 		LPT_INFO("cout", "o_true: " << o_true);
// 		LPT_INFO("cout", "object_id::FALSE: " << object_id::FALSE);
// 		LPT_INFO("cout", "object_id::TRUE: " << object_id::TRUE);
// 		assert(o_false == object_id::FALSE);
// 		assert(o_true == object_id::TRUE);
		
		bind_object_to_type(bool_t, o_false);
		bind_object_to_type(bool_t, o_true);
	}
	
	Implementation(const Implementation&) = default;
	Implementation(Implementation&&) = default;
	Implementation& operator=(const Implementation&) = default;
	Implementation& operator=(Implementation&&) = default;
	
	
	inline TypeIdx get_fstype_id(const std::string& type_name) const {
		auto it = _name_to_type.find(type_name);
		if (it == _name_to_type.end()) {
			throw std::runtime_error("Unknown object type " + type_name);
		}
		return it->second;
	}	
	
	
	SymbolIdx get_symbol_id(const std::string& name) const {
		const auto& it = symbolIds.find(name);
		if (it == symbolIds.end()) {
			throw unregistered_symbol("Symbol \"" + name + "\" has not been registered");
		}		
		return symbolIds.at(name);
	}

	const std::string& get_symbol_name(symbol_id symbol) const { return symbolinfo(symbol).name(); }
	
	type_id get_type_id(const std::string& fstype) const {
		if (fstype == "bool") return type_id::bool_t;
		return get_type_id(get_fstype_id(fstype));
	}

	
	const std::string get_typename(const type_id& type) const {
		return to_string(type);
	}
	
	type_id get_type_id(TypeIdx fstype) const { return typeinfo(fstype).get_type_id(); }
	const std::string& get_typename(const TypeIdx& fstype) const { return typeinfo(fstype).name(); }
	
	void _check_valid_fstype(const TypeIdx& fstype) const {
		if (fstype >= _fstype_info.size()) {
			throw unregistered_type("FS-Type with id \"" + std::to_string(fstype) + "\" has not been registered");
		}
	}
	
	const FSTypeInfo& typeinfo(const TypeIdx& fstype) const {
		_check_valid_fstype(fstype);
		return _fstype_info.at(fstype);
	}
	
	const SymbolInfo& symbolinfo(const symbol_id& symbol) const {
		if (symbol >= _symbol_info.size()) {
			throw unregistered_symbol("Symbol with id \"" + std::to_string(symbol) + "\" has not been registered");
		}
		return _symbol_info[symbol];		
	}
	
	
	const std::string get_object_name(const object_id& object) const {
		type_id t = o_type(object);
		if (t == type_id::bool_t) return std::string((fs0::value<bool>(object) ? "true" : "false"));
		else if (t == type_id::int_t) return std::to_string(fs0::value<int>(object));
		else if (t == type_id::float_t) return std::to_string(fs0::value<float>(object));
		else if (t == type_id::object_t) return get_custom_object_name(object);
		throw unregistered_type("Unknown type_id \"" + to_string(t) + "\"");
	}
	
	const std::string& get_custom_object_name(const object_id& object) const {
		assert(o_type(object) == type_id::object_t);
		const auto& it = _object_names.find(object);
		if (it == _object_names.end()) throw unregistered_object("Unregistered object");
		return it->second;		
	}	
	
	symbol_id add_symbol(const std::string& name, const symbol_t& type, const Signature& signature, bool static_) {
		assert(_symbol_info.size() == symbolIds.size());
		unsigned id = symbolIds.size();
		symbolIds.insert(std::make_pair(name, id));
		_symbol_info.push_back(SymbolInfo(id, type, name, signature, static_));
		return id;
	}
	
	TypeIdx add_fstype(const std::string& name) { 
		TypeIdx id = _add_fstype(name, type_id::object_t);
		_type_ranges.push_back(INVALID_TYPE_RANGE); // i.e. the type has no actual associated range
		return id;
	}
	
	TypeIdx add_fstype(const std::string& name, type_id underlying_type, const type_range& range) {
		type_id t = check_valid_range(range);
		if (underlying_type != t) throw range_type_mismatch(underlying_type, t);
		
		TypeIdx id = _add_fstype(name, underlying_type);
		_type_ranges.push_back(range);
		
		if (t != type_id::object_t) {
			fill_object_range(id, range);
		}
		
		return id;
	}
	
	//! Private method - Add a fs-type with given underlying type_id
	TypeIdx _add_fstype(const std::string& name, type_id underlying_type) {
		assert(_fstype_info.size() == _name_to_type.size());
		assert(_fstype_info.size() == _fstype_objects.size());
		assert(_fstype_info.size() == _type_ranges.size());
		
		TypeIdx id = _fstype_info.size();
		_name_to_type.insert(std::make_pair(name, id));
		_fstype_objects.push_back(std::vector<object_id>()); // Push back an empty vector
		
		
		
		_fstype_info.push_back(FSTypeInfo(id, name, underlying_type));
		
		return id;
	}	
	
	//! Check that the range is valid and return its type
	type_id check_valid_range(const type_range& range) const {
		const object_id& min = range.first;
		const object_id& max = range.second;
		type_id t = o_type(max);
		if (o_type(max) != o_type(min)) throw range_type_mismatch(o_type(min), o_type(max));
		
		if (t == type_id::int_t && value<int>(min) >= value<int>(max)) {
			throw invalid_range("Invalid range: [" + std::to_string(value<int>(min)) + " - " + std::to_string(value<int>(max)) + "}");
		}
		// TODO - else if type is float ... 
		
		return t;
	}
	
	void fill_object_range(TypeIdx type, const type_range& range) {
		if (o_type(range.first) == type_id::int_t) {
			std::vector<object_id> range_values = generate_object_range<int>(range);
			std::vector<object_id>& type_objs = _fstype_objects.at(type);
			type_objs.insert(type_objs.end(), range_values.begin(), range_values.end() );
		}
		// TODO - else if type is float ... 
	}
	
	template <typename T>
	std::vector<object_id> generate_object_range(const type_range& range) const {
		std::vector<object_id> result;
		const object_id& min = range.first;
		const object_id& max = range.second;
		
		const T& valmin = value<T>(min);
		const T& valmax = value<T>(max);
		
		for (T v = valmin; v <= valmax; ++v) {
			result.push_back(make_object<T>(v));
		}
		
		return result;
	}

	object_id add_object(const std::string& name, TypeIdx fstype) {
		assert(_object_ids.size() == _object_names.size());
		unsigned id = _object_names.size();
		type_id t = get_type_id(fstype);
		
		if (t != type_id::object_t) throw std::runtime_error("Cannot add non-object types");
		
		object_id oid = make_object(type_id::object_t, id);
		
		_object_ids.insert(std::make_pair(name, oid));
		_object_names.insert(std::make_pair(oid, name));
		
// 		_fstype_objects.at(fstype).push_back(oid);
		
		return oid;
	}
	
	unsigned num_objects() const { return _object_names.size(); }
	
	void bind_object_to_type(TypeIdx fstype, object_id object) {
		assert(o_type(object) == type_id::object_t);
		_fstype_objects.at(fstype).push_back(object);
	}
	
	const std::vector<object_id>& type_objects(TypeIdx fstype) const {
		_check_valid_fstype(fstype);
		return _fstype_objects.at(fstype);
	}
	
	
	
	//! The bounds of bounded types, e.g. bounded integer types
	std::vector<type_range> _type_ranges;
	
	//! Map between predicate and function symbols names and IDs.
	std::unordered_map<std::string, SymbolIdx> symbolIds;
	
	//! Map between (FSTRIPS) typenames and type IDs.
	std::unordered_map<std::string, TypeIdx> _name_to_type;
	
	//! Map from object index to object name
	std::unordered_map<object_id, std::string> _object_names;
	//! Map from object name to object index
	std::unordered_map<std::string, object_id> _object_ids;
	
	//! Map from fs-type ID to all of the object indexes of that type
	std::vector<std::vector<object_id>> _fstype_objects;
	
	
	std::vector<FSTypeInfo> _fstype_info;
	
	std::vector<SymbolInfo> _symbol_info;
	
};

LanguageInfo::LanguageInfo() : _impl(new Implementation()) {}

LanguageInfo::~LanguageInfo() = default;
LanguageInfo::LanguageInfo(LanguageInfo&&) = default;
LanguageInfo& LanguageInfo::operator=(LanguageInfo&&) = default;

LanguageInfo::LanguageInfo(const LanguageInfo& other) 
	: _impl(new Implementation(*other._impl))
{}

LanguageInfo& LanguageInfo::operator=(const LanguageInfo& other) {
    if (this != &other) {
        _impl.reset(new Implementation(*other._impl));
    }
    return *this;
}


SymbolIdx LanguageInfo::
get_symbol_id(const std::string& name) const { return impl().get_symbol_id(name); }

const std::string& LanguageInfo::
get_symbol_name(symbol_id symbol) const { return impl().get_symbol_name(symbol); }

const std::vector<SymbolInfo>& LanguageInfo::
all_symbols() const { return impl()._symbol_info; }

unsigned LanguageInfo::
num_symbols() const { return impl().symbolIds.size(); }

TypeIdx LanguageInfo::
get_fstype_id(const std::string& fstype) const { return impl().get_fstype_id(fstype); }

type_id LanguageInfo::
get_type_id(const std::string& fstype) const { return impl().get_type_id(fstype); }

type_id LanguageInfo::
get_type_id(TypeIdx fstype) const { return impl().get_type_id(fstype); }



const std::string LanguageInfo::
get_typename(const type_id& type) const { return impl().get_typename(type); }

const std::string& LanguageInfo::
get_typename(const TypeIdx& fstype) const { return impl().get_typename(fstype); }

const std::string LanguageInfo::
get_object_name(const object_id& object) const { return impl().get_object_name(object); }

symbol_id LanguageInfo::
add_symbol(const std::string& name, const symbol_t& type, const Signature& signature, bool static_) { return impl().add_symbol(name, type, signature, static_); }

object_id LanguageInfo::
add_object(const std::string& name, TypeIdx fstype) { return impl().add_object(name, fstype); }

unsigned LanguageInfo::
num_objects() const { return impl().num_objects(); }


TypeIdx LanguageInfo::
add_fstype(const std::string& name) { return impl().add_fstype(name); }

TypeIdx LanguageInfo::
add_fstype(const std::string& name, type_id underlying_type, const type_range& range) { return impl().add_fstype(name, underlying_type, range); }

void LanguageInfo::
bind_object_to_type(TypeIdx fstype, object_id object) { return impl().bind_object_to_type(fstype, object); }

const FSTypeInfo& LanguageInfo::
typeinfo(const TypeIdx& fstype) const { return impl().typeinfo(fstype); }

const SymbolInfo& LanguageInfo::
symbolinfo(const symbol_id& fstype) const { return impl().symbolinfo(fstype); }

const std::vector<object_id>& LanguageInfo::
type_objects(TypeIdx fstype) const { return impl().type_objects(fstype); }

} } // namespaces
