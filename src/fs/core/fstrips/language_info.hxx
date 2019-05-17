
#pragma once

#include <utility>
#include <memory>
#include <fs/core/fs_types.hxx> // TODO - REMOVE DEPENDENCY?


namespace fs0::fstrips {

using symbol_id = unsigned;
enum class symbol_t {Predicate, Function};


//! A Functional STRIPS type, such as "block", containing a given ID, a name, and the type of the underlying
//! primitive type used to represent the FS type, e.g. an int, in the case of "block".
//! FSTypes can optionally have bounds.
class FSType {
public:
	FSType(TypeIdx id, std::string name_, type_id underlying_type, type_range bounds)
		: _id(id), _name(std::move(name_)), _type_id(underlying_type), _bounds(std::move(bounds))
	{}

	//! Return the unique ID used for this type
	const TypeIdx& id() const { return _id; }

	//! Return the name of the Functional STRIPS type, e.g. "block", or "cell"
	const std::string& name() const { return _name; }

	//! Return the ID of the underlying primitive type
	type_id get_type_id() const { return _type_id; }

	//! Whether the type is bounded
	bool bounded() const { return _bounds != INVALID_TYPE_RANGE; }

	//! Return the bounds of the type, if available
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

//! A FOL function or predicate symbol
class SymbolInfo {
public:
	SymbolInfo(symbol_id id, const symbol_t& symbol_type, std::string name_, Signature signature)
		: _id(id), _symbol_type(symbol_type), _name(std::move(name_)), _signature(std::move(signature))
	{}

	//! Return the ID of the symbol
	const symbol_id& id() const { return _id; }

	//! Return the type of symbol, i.e. predicate or function
	symbol_t type() const { return _symbol_type; }

	//! Return the name of the predicate or function
	const std::string& name() const  { return _name; }

	//! Return the sort of the symbol, i.e. a tuple with arity equal to that of the symbol, +1 if the symbol is a
	//! function. The sort of predicate symbol "clear" in blocksworld, for instance, might be (block), whereas that of
	//! function symbol "loc" in the FSTRIPS version of blocks is (block, place)
	const Signature& signature() const { return _signature; }

	//! Return the arity of the predicate or function
	unsigned arity() const {
	    auto ss = _signature.size();
	    return _symbol_type == symbol_t::Predicate ? ss : ss-1;
	}

	//! Return the size of the sort of the symbol. For predicates, it is the same as the arity; for functions, it
	//! is the arity + 1
    unsigned uniform_arity() const {
        return _signature.size();
    }


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

	//! Add a new predicate or function symbol with the given signature
    symbol_id add_symbol(const std::string& name, const symbol_t& type, const Signature& signature);

    //! Return the symbol info associated to the given symbol
    const SymbolInfo& symbolinfo(const symbol_id& sid) const;


    //! Return the total number of FStrips types
    std::size_t num_fs_types() const;

    //! Add FStrips types, bounded or unbounded
    TypeIdx add_fs_type(const std::string& name, type_id underlying_type);
    TypeIdx add_fs_type(const std::string& name, type_id underlying_type, const type_range& range);

	//! Return the ID of the FStrips type with given name
	TypeIdx get_fs_type_id(const std::string& fs_type) const;

	//! Return the generic type_id corresponding to the given FS type id or name
	type_id get_type_id(const std::string& fs_type) const;
	type_id get_type_id(TypeIdx fs_type) const;


    //! Return the typename of a primitive type, e.g. "int" or "bool"
	const std::string get_typename(const type_id& type) const;

	//! Return the typename of a FSTRIPS type, e.g. "block"
	const std::string& get_typename(const TypeIdx& fs_type) const;


    //! Return the number of PDDL objects
    unsigned num_objects() const;

    //! Check that the given object is within the bounds of its type; if it is not, throw out_of_range_object.
    void check_valid_object(const object_id& object, TypeIdx type) const;

	const std::string get_object_name(const object_id& object) const;

	const object_id get_object_id(const std::string& name) const;

    //! Add an object with the given name and PDDL type
	object_id add_object(const std::string& name, TypeIdx pddl_type);



	//! Declare that the given object belongs to the given type. One object can belong to more than
	//! one type (as per the standard PDDL type hierarchies)
	void bind_object_to_fs_type(TypeIdx fs_type, object_id object);

	//! Return the typeinfo associated to the given fs-type
	const FSType& typeinfo(const TypeIdx& fs_type) const;


	//! Return all objects of a given FS-type, _including_ those which are
	//! objects of a descending type in the type hierarchy
	const std::vector<object_id>& type_objects(TypeIdx fs_type) const;

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
