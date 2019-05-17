#include <utility>

#include <utility>


#pragma once

#include <memory>
#include <fs/core/fs_types.hxx> // TODO - REMOVE DEPENDENCY?


namespace fs0::fstrips {

using symbol_id = unsigned;
enum class symbol_t {Predicate, Function};


class PrimitiveType {
public:
	PrimitiveType(TypeIdx id, std::string name_, type_id underlying_type, type_range bounds)
		: _id(id), _name(std::move(name_)), _type_id(underlying_type), _bounds(std::move(bounds))
	{}

	//!
	const TypeIdx& id() const { return _id; }

	//!
	const std::string& name() const  { return _name; }

	//!
	type_id get_type_id() const { return _type_id; }

	//!
	bool bounded() const { return _bounds != INVALID_TYPE_RANGE; }

	template <typename T>
	const std::pair<T, T> bounds() const {
		return std::make_pair<T>(value<T>(_bounds.first), value<T>(_bounds.second));
	}

protected:
	//!
	TypeIdx _id;

	//!
	const std::string _name;

	//!
	type_id _type_id;

	//!
	type_range _bounds;
};

class SymbolInfo {
public:
	SymbolInfo(symbol_id id, const symbol_t& symbol_type, std::string name_, Signature signature)
		: _id(id), _symbol_type(symbol_type), _name(std::move(name_)), _signature(std::move(signature))
	{}

	//!
	const symbol_id& id() const { return _id; }

	//!
	symbol_t type() const { return _symbol_type; }

	//!
	const std::string& name() const  { return _name; }

	//!
	const Signature& signature() const { return _signature; }


protected:
	//!
	symbol_id _id;

	//!
	symbol_t _symbol_type;

	//!
	const std::string _name;

	//!
	const Signature _signature;
};


class LanguageInfo {
public:
	LanguageInfo();
	~LanguageInfo();
	LanguageInfo(const LanguageInfo&);
	LanguageInfo(LanguageInfo&&);
	LanguageInfo& operator=(const LanguageInfo&);
	LanguageInfo& operator=(LanguageInfo&&);


    //! Return all symbol names
    const std::vector<SymbolInfo>& all_symbols() const;

    //! Return the total number of predicate / function symbols
    std::size_t num_symbols() const;

	//! Return the ID of the predicate / function symbol with the given name
	SymbolIdx get_symbol_id(const std::string& name) const;

	//! Return the name of the predicate / function symbol with the given id
	const std::string& get_symbol_name(symbol_id symbol) const;


    //! Return the total number of primitive types
    std::size_t num_primitive_types() const;

	//! Return the ID of the fs-type with given name
	TypeIdx get_primitive_type_id(const std::string& primitive_type) const;

	//! Return the generic type_id corresponding to the given fs-type
	type_id get_type_id(const std::string& primitive_type) const;
	type_id get_type_id(TypeIdx primitive_type) const;

	void check_valid_object(const object_id& object, TypeIdx type) const;

	const std::string get_typename(const type_id& type) const;
	const std::string& get_typename(const TypeIdx& primitive_type) const;

	const std::string get_object_name(const object_id& object) const;

	const object_id get_object_id(const std::string& name) const;

	//! Return the number of registered objects
	unsigned num_objects() const;


	symbol_id add_symbol(const std::string& name, const symbol_t& type, const Signature& signature);


	object_id add_object(const std::string& name, TypeIdx primitive_type);

	TypeIdx add_primitive_type(const std::string& name, type_id underlying_type);
	TypeIdx add_primitive_type(const std::string& name, type_id underlying_type, const type_range& range);

	//! TODO - DEPRECATE. Object and Types should be implicitly bound by means of type hierarchy.
	void bind_object_to_type(TypeIdx primitive_type, object_id object);

	//! Return the typeinfo associated to the given fs-type
	const PrimitiveType& typeinfo(const TypeIdx& primitive_type) const;

	//! Return the symbol info associated to the given symbol
	const SymbolInfo& symbolinfo(const symbol_id& sid) const;

	//! Return all objects of a given FS-type, _including_ those which are
	//! objects of a descending type in the type hierarchy
	const std::vector<object_id>& type_objects(TypeIdx primitive_type) const;

    //! Print a representation of the object to the given stream.
    friend std::ostream& operator<<(std::ostream &os, const LanguageInfo& o) { return o.print(os); }
    std::ostream& print(std::ostream& os) const;

	// ************************************************
	// TODO THIS IS A TEMPORARY WORKAROUND
	// TODO REMOVE THIS.
	// Object printers are the main hindrance for that... we might want to put a pointer
	// to the LanguageInfo object in each AST node to get rid of that?
	// Or, alternatively, get rid of object printers and have generic printers in fs0::print
	// that require a LanguageInfo object to print the actual AST node.
	// (i.e. one way or another, an AST node cannot be printed without the info contained in
	// the corresponding LanguageInfo object)
	//! The singleton instance
	static std::unique_ptr<LanguageInfo> _instance;

	//! Set the singleton instance
	static LanguageInfo& instance(LanguageInfo* info) {
		assert(!_instance);
		_instance = std::unique_ptr<LanguageInfo>(info);
		return *_instance;
	}

	//! Singleton object accessor
	static const LanguageInfo& instance() {
		assert(_instance);
		return *_instance;
	}
	// ************************************************

private:
	// Pimpl idiom
	class Implementation;
	std::unique_ptr<Implementation> _impl;
	Implementation& impl() const { return *_impl; };
};


} // namespaces
