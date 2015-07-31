
#include <constraints/gecode/translator_repository.hxx>
#include <iostream>

namespace fs0 { namespace gecode {

TranslatorRepository::TranslatorRepository() {}

TranslatorRepository::~TranslatorRepository() {
	for (auto& it:_constraintTranslators) delete it.second;
	for (auto& it:_effectTranslators) delete it.second;
}

TranslatorRepository&
TranslatorRepository::instance() {
	static TranslatorRepository  theInstance;
	return theInstance;
}

void
TranslatorRepository::addEntry( TypeInfoRef typeThing, ConstraintTranslator::ptr componentTranslator ) {
	_constraintTranslators[ typeThing ] = componentTranslator;
}

void
TranslatorRepository::addEntry( TypeInfoRef typeThing, EffectTranslator::ptr componentTranslator ) {
	_effectTranslators[ typeThing ] = componentTranslator;
}

EffectTranslator::ptr
TranslatorRepository::getEffectTranslator( TypeInfoRef type ) {
	auto it = _effectTranslators.find( type );
	if ( it == _effectTranslators.end() ) return nullptr;
	return it->second;
}

ConstraintTranslator::ptr
TranslatorRepository::getConstraintTranslator( TypeInfoRef type ) {
	auto it = _constraintTranslators.find( type );
	if ( it == _constraintTranslators.end() ) return nullptr;
	return it->second;
}


} } // namespaces