
#include <constraints/gecode/utils/translation.hxx>
#include <boost/functional/hash.hpp>
#include <languages/fstrips/terms.hxx>

namespace fs0 { namespace gecode {

TranslationKey::TranslationKey(const fs::Term* term, CSPVariableType _type)
	: _term(term), _type(_type) {}

bool TranslationKey::operator==(const TranslationKey& rhs) const {
	return _type == rhs._type && *_term == *rhs._term;
}

bool TranslationKey::operator!=(const TranslationKey& rhs) const { return ! this->operator==(rhs); }

std::size_t TranslationKey::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, _type);
	boost::hash_combine(hash, _term->hash_code());
	return hash;
}

} } // namespaces
