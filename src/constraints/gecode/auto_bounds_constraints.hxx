#ifndef __AUTO_BOUNDS_CONSTRAINTS__
#define __AUTO_BOUNDS_CONSTRAINTS__

#include <constraints/gecode/expr_translators.hxx>

namespace fs0 {

  namespace gecode {

    class UnaryDomainBoundsConstraintTranslator : public ConstraintTranslator  {
    public:

      virtual ~UnaryDomainBoundsConstraintTranslator();
      virtual void addConstraint( ScopedConstraint::cptr constr, ComplexActionManager& manager ) const;
    };

    class BinaryDomainBoundsConstraintTranslator : public ConstraintTranslator  {
    public:

      virtual ~BinaryDomainBoundsConstraintTranslator();
      virtual void addConstraint( ScopedConstraint::cptr constr, ComplexActionManager& manager ) const;
    };


  }

}

#endif // auto_bounds_constraints.hxx
