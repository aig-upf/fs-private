

#pragma once

#include <languages/fstrips/language.hxx>
#include <boost/functional/hash.hpp>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

//! The possible types of CSP variables that we might consider
enum class CSPVariableType {
	Input, // a variable relevant to some formula (in an action precondition, goal, etc.)
	Output, // a variable affected by some action effect
};


//!
struct TranslationKey {
	
	TranslationKey(fs::Term::cptr term, CSPVariableType _type)
		: _term(term), _type(_type) {}

	inline bool operator==(const TranslationKey& rhs) const {
		return _type == rhs._type && *_term == *rhs._term;
	}

	inline bool operator!=(const TranslationKey& rhs) const { return ! this->operator==(rhs); }

// 	inline bool operator<(const TranslationKey& rhs) const {
// 		return _type < rhs._type || (_type == rhs._type && *_term < *rhs._term);
// 	}

// 	inline bool operator<=(const TranslationKey& rhs) const { return !this->operator>(rhs); }

// 	inline bool operator>(const TranslationKey& rhs) const { return rhs.operator<(*this); }

// 	inline bool operator>=(const TranslationKey& rhs) const { return !this->operator<(rhs); }

	std::size_t hash_code() const {
		std::size_t hash = 0;
		boost::hash_combine(hash, _type);
		boost::hash_combine(hash, _term->hash_code());
		return hash;
	}

	const fs::Term::cptr getTerm() const { return _term; }
	CSPVariableType getType() const { return _type; }
	
protected:
	//! The actual term being indexed
	fs::Term::cptr _term;
	
	//! The role under which the term is indexed
	CSPVariableType _type;
};


} } // namespaces

namespace std { // std::hash specialization for TranslationKey.
	
	template<>
	struct hash<fs0::gecode::TranslationKey>
	{
		typedef std::size_t        result_type;
		typedef fs0::gecode::TranslationKey  argument_type;

		std::size_t operator()(const fs0::gecode::TranslationKey& elem) const noexcept { return elem.hash_code(); }
	};
} // namespace std	