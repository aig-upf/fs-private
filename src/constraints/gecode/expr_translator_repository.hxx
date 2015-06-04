#ifndef __EXPR_TRANSLATOR_Repository__
#define __EXPR_TRANSLATOR_Repository__

#include <unordered_map>
#include <utils/typeinfo_helper.hxx>
#include <constraints/gecode/expr_translators.hxx>

namespace fs0 {

  namespace gecode {

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

      void                            register( TypeInfoRef type, ConstraintTranslator::ptr obj);
      void                            register( TypeInfoRef type, EffectTranslator::ptr obj);

      EffectTranslator::ptr           getEffectTranslator( TypeInfoRef type );
      ConstraintTranslator::ptr       getConstraintTranslator( TypeInfoRef type );


    protected:

      TranslatorRepository();

      ConstraintTranslatorTable   _constraintTranslators;
      EffectTranslatorTable       _effectTranslators;

    };

    template <typename TranslatorType >
    class TranslatorRegistrar {
    public :
      TranslatorRegistrar( TypeInfoRef k ) {
        TranslatorRepository::instance().register( k, new TranslatorType );
      }
    };

  }

}

#endif // expr_translator_Repository.hxx
