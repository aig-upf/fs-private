
#pragma once

#include <memory>
#include <fs_types.hxx>


namespace fs0 { namespace fstrips {

class LanguageInfo {
public:

	LanguageInfo();
	~LanguageInfo();	
	
	
	//! Return the total number of predicate / function symbols
	unsigned num_symbols() const;
	
	//! Return the ID of the predicate / function symbol with the given name
	SymbolIdx get_symbol_id(const std::string& name) const;
	
	//! Return the name of the predicate / function symbol with the given id
	const std::string& get_symbol_name(unsigned symbol_id) const;
	
	//! Return all symbol names
	const std::vector<std::string>& all_symbol_names() const;
	
	
	//! Return the ID of the fs-type with given name
	TypeIdx get_fstype_id(const std::string& fstype) const;
	
	//! Return the generic type_id corresponding to the given fs-type
	type_id get_type_id(const std::string& fstype) const;
	type_id get_type_id(TypeIdx fstype) const;	
	
private:
	// Pimpl idiom
	class LanguageInfoImpl;
	const std::unique_ptr<LanguageInfoImpl> _impl;
	const LanguageInfoImpl& impl() const { return *_impl; };
};

} } // namespaces
