#include <constraints/gecode/expr_translator_repository.hxx>
#include <iostream>

namespace fs0 { namespace gecode {

    TranslatorRepository::TranslatorRepository() {

    }

    TranslatorRepository::~TranslatorRepository() {

    }

    TranslatorRepository&
    TranslatorRepository::instance() {
      static TranslatorRepository  theInstance;
      return theInstance;
    }

    void
    TranslatorRepository::addEntry( TypeInfoRef typeThing, ConstraintTranslator::ptr transObj ) {
      _constraintTranslators[ typeThing ] = transObj;
    }

    void
    TranslatorRepository::addEntry( TypeInfoRef typeThing, EffectTranslator::ptr transObj ) {
      _effectTranslators[ typeThing ] = transObj;
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