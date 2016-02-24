
#pragma once

#include <functional>

namespace fs0 { namespace language { namespace fstrips { class Term; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

//! The possible types of CSP variables that we might consider
enum class CSPVariableType {
	Input, // a variable relevant to some formula (in an action precondition, goal, etc.)
	Output, // a variable affected by some action effect
};


struct TranslationKey {

	TranslationKey(const fs::Term* term, CSPVariableType _type);

	bool operator==(const TranslationKey& rhs) const;
	bool operator!=(const TranslationKey& rhs) const;

	std::size_t hash_code() const;

	const fs::Term* getTerm() const { return _term; }
	CSPVariableType getType() const { return _type; }

protected:
	//! The actual term being indexed
	const fs::Term* _term;

	//! The role under which the term is indexed
	CSPVariableType _type;
};


} } // namespaces

// std::hash specialization for TranslationKey.
namespace std {
	template<> struct hash<fs0::gecode::TranslationKey> {
		typedef std::size_t result_type;
		typedef fs0::gecode::TranslationKey argument_type;

		std::size_t operator()(const fs0::gecode::TranslationKey& elem) const noexcept { return elem.hash_code(); }
	};
} // namespace std
