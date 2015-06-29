#ifndef __EXPR_TRANSLATORS__
#define __EXPR_TRANSLATORS__

#include <constraints/scoped_constraint.hxx>
#include <constraints/scoped_effect.hxx>
#include <gecode/driver.hh>

namespace fs0 {

  namespace gecode {

    class ActionCSP;

    class ConstraintTranslator  {
    public:

      typedef ConstraintTranslator  *   ptr;

      virtual ~ConstraintTranslator();

      virtual void addConstraint( ScopedConstraint::cptr constr, ActionCSP& s ) const = 0;
    };

    class EffectTranslator  {
    public:

      typedef EffectTranslator  *   ptr;

      virtual ~EffectTranslator();

      virtual void addConstraint( ScopedEffect::cptr eff,  ActionCSP& s ) const = 0;
    };


  }

}

#endif // expr_translators.hxx
