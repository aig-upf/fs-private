
#pragma once

#include <unordered_map>
#include <utils/typeinfo_helper.hxx>
#include <constraints/gecode/translators.hxx>

namespace fs0 { namespace gecode {

class TranslatorRepository {
public:
	typedef   std::unordered_map<   TypeInfoRef,
									ConstraintTranslator::ptr,
									TypeInfoHasher,
									TypeInfoEqualsTo > ConstraintTranslatorTable;
	typedef   std::unordered_map<   TypeInfoRef,
									EffectTranslator::ptr,
									TypeInfoHasher,
									TypeInfoEqualsTo > EffectTranslatorTable;

	~TranslatorRepository();

	static  TranslatorRepository&   instance();
	void                            addEntry(TypeInfoRef type, ConstraintTranslator::ptr componentTranslator);
	void                            addEntry(TypeInfoRef type, EffectTranslator::ptr componentTranslator);

	EffectTranslator::ptr           getEffectTranslator( TypeInfoRef type );
	ConstraintTranslator::ptr       getConstraintTranslator( TypeInfoRef type );


protected:
	TranslatorRepository();

	ConstraintTranslatorTable   _constraintTranslators;
	EffectTranslatorTable       _effectTranslators;
};

} } // namespaces
