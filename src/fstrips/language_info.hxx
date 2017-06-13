
#pragma once

#include <memory>
#include <fs_types.hxx> // TODO - REMOVE DEPENDENCY?


namespace fs0 { namespace fstrips {

using symbol_id = unsigned;
enum class symbol_t {Predicate, Function};

class LanguageInfo {
public:

	LanguageInfo();
	~LanguageInfo();
	LanguageInfo(const LanguageInfo& other);
	LanguageInfo(LanguageInfo&& other);
	LanguageInfo& operator=(const LanguageInfo& other);
	LanguageInfo& operator=(LanguageInfo&& other);
	
	
	//! Return the ID of the predicate / function symbol with the given name
	SymbolIdx get_symbol_id(const std::string& name) const;
	
	//! Return the name of the predicate / function symbol with the given id
	const std::string& get_symbol_name(symbol_id symbol) const;
	
	//! Return all symbol names
	const std::vector<std::string>& all_symbol_names() const;
	
	//! Return the total number of predicate / function symbols
	unsigned num_symbols() const;
	
	
	
	//! Return the ID of the fs-type with given name
	TypeIdx get_fstype_id(const std::string& fstype) const;
	
	//! Return the generic type_id corresponding to the given fs-type
	type_id get_type_id(const std::string& fstype) const;
	type_id get_type_id(TypeIdx fstype) const;


	
	const std::string get_typename(const type_id& type) const;
	const std::string& get_typename(const TypeIdx& fstype) const;
	
	const std::string get_object_name(const object_id& object) const;
	
	//! Return the number of registered objects
	unsigned num_objects() const;
	
	
	symbol_id add_symbol(const std::string& name, const symbol_t& type, const Signature& signature);
	
	
	object_id add_object(const std::string& name, TypeIdx fstype);
	
	TypeIdx add_fstype(const std::string& name);
	TypeIdx add_fstype(const std::string& name, type_id underlying_type, const type_range& range);
		
	//! TODO - DEPRECATE. Object and Types should be implicitly bound by means of type hierarchy.
	void bind_object_to_type(TypeIdx fstype, object_id object);

	
	
private:
	// Pimpl idiom
	class Implementation;
	std::unique_ptr<Implementation> _impl;
	Implementation& impl() const { return *_impl; };
};

class LanguageJsonLoader {
public:
	static LanguageInfo loadLanguageInfo(const std::string& filename);
};

} } // namespaces
