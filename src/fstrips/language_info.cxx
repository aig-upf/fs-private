
#include <unordered_map>

#include <fstrips/language_info.hxx>


namespace fs0 { namespace fstrips {
	
class unregistered_symbol : public std::runtime_error {
public:
	unregistered_symbol(const std::string& msg) : std::runtime_error(msg) {}
};

	
	
class LanguageInfo::LanguageInfoImpl {
private:
	LanguageInfoImpl() {}
	
	friend class LanguageInfo;
	
	inline TypeIdx get_fstype_id(const std::string& type_name) const {
		auto it = name_to_type.find(type_name);
		if (it == name_to_type.end()) {
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

	const std::string& get_symbol_name(unsigned symbol_id) const {
		if (symbol_id >= symbolNames.size()) {
			throw unregistered_symbol("Symbol with id \"" + std::to_string(symbol_id) + "\" has not been registered");
		}
		return symbolNames[symbol_id];
	}
	
	
	
	
	//! Maps between predicate and function symbols names and IDs.
	std::vector<std::string> symbolNames;
	std::unordered_map<std::string, SymbolIdx> symbolIds;
	
	//! Maps between typenames and type IDs.
	std::unordered_map<std::string, TypeIdx> name_to_type;
	std::vector<std::string> type_to_name;	
};




LanguageInfo::LanguageInfo() : _impl(new LanguageInfoImpl()) {}

LanguageInfo::~LanguageInfo() = default;

SymbolIdx LanguageInfo::
get_symbol_id(const std::string& name) const { return impl().get_symbol_id(name); }

const std::string& LanguageInfo::
get_symbol_name(unsigned symbol_id) const { return impl().get_symbol_name(symbol_id); }

const std::vector<std::string>& LanguageInfo::
all_symbol_names() const { return impl().symbolNames; }

unsigned LanguageInfo::
num_symbols() const { return impl().symbolIds.size(); }

TypeIdx LanguageInfo::
get_fstype_id(const std::string& fstype) const { return impl().get_fstype_id(fstype); }


} } // namespaces
