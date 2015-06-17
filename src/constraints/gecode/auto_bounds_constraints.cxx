#include <constraints/gecode/auto_bounds_constraints.hxx>
#include <constraints/gecode/support_csp.hxx>
#include <constraints/scoped_constraint.hxx>
#include <constraints/gecode/expr_translator_repository.hxx>
#include <cassert>

namespace fs0 {

  namespace gecode {

    UnaryDomainBoundsConstraintTranslator::~UnaryDomainBoundsConstraintTranslator() {

    }

    void
    UnaryDomainBoundsConstraintTranslator::addConstraint(ScopedConstraint::cptr c, SupportCSP& csp ) const {
      auto* con = dynamic_cast<UnaryDomainBoundsConstraint*>( c );
      assert( con != nullptr);
      csp.addBoundsConstraintFromDomain( con->getAffected() );
    }

    BinaryDomainBoundsConstraintTranslator::~BinaryDomainBoundsConstraintTranslator() {

    }

    void
    BinaryDomainBoundsConstraintTranslator::addConstraint( ScopedConstraint::cptr c, SupportCSP& csp ) const {
      auto* con = dynamic_cast<BinaryDomainBoundsConstraint*>( c );
      assert( con != nullptr);
      csp.addBoundsConstraintFromDomain( con->getAffected() );
    }


  }

}
