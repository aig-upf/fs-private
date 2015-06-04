#include <constraints/gecode/expr_translator_repository.hxx>

namespace fs0 {

  namespace gecode {

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
    TranslatorRepository::register( TypeInfoRef typeThing, ConstraintTranslator::ptr transObj ) {
      _constraintTranslators[ typeThing ] = transObj;
    }

    void
    TranslatorRepository::register( TypeInfoRef typeThing, EffectTranslator::ptr transObj ) {
      _effectTranslators[ typeThing ] = transObj;
    }

    EffectTranslator::ptr
    TranslatorRepository::getEffectTranslator( TypeInfoRef type ) {
      auto it = _effectTranslator.find( type );
      if ( it == _effectTranslator.end() ) return nullptr;
      return it->second;
    }

    ConstraintTranslator::ptr
    TranslatorRepository::getConstraintTranslator( TypeInfoRef type ) {
      auto it = _constraintTranslator.find( type );
      if ( it == _constraintTranslator.end() ) return nullptr;
      return it->second;
    }


  }

}
