
#pragma once

#include <unordered_map>
#include <utils/typeinfo_helper.hxx>
#include <constraints/gecode/expr_translators.hxx>

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
      void                            addEntry( TypeInfoRef type, ConstraintTranslator::ptr obj);
      void                            addEntry( TypeInfoRef type, EffectTranslator::ptr obj);

      EffectTranslator::ptr           getEffectTranslator( TypeInfoRef type );
      ConstraintTranslator::ptr       getConstraintTranslator( TypeInfoRef type );


    protected:

      TranslatorRepository();

      ConstraintTranslatorTable   _constraintTranslators;
      EffectTranslatorTable       _effectTranslators;

    };

    // MRJ: Having to split the registrar into two types is highly unsatisfying
    // but I see that g++ can't resolve overloaded methods when there is and
    // implicit upcast (which is kind of reasonable, I guess)
    template <typename TranslatorType >
    class EffectTranslatorRegistrar {
    public :
      EffectTranslatorRegistrar( TypeInfoRef k ) {
        TranslatorRepository::instance().addEntry( k, (EffectTranslator::ptr)(new TranslatorType()) );
      }
    };

    template <typename TranslatorType >
    class ConstraintTranslatorRegistrar {
    public :
      ConstraintTranslatorRegistrar( TypeInfoRef k ) {
        TranslatorRepository::instance().addEntry( k, (ConstraintTranslator::ptr)(new TranslatorType()) );
      }
    };

} } // namespaces
