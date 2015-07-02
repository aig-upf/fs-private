#include <constraints/gecode/auto_bounds_constraints.hxx>
#include <constraints/scoped_constraint.hxx>
#include <constraints/gecode/expr_translator_repository.hxx>
#include <constraints/gecode/helper.hxx>
#include <cassert>

namespace fs0 { namespace gecode {

    UnaryDomainBoundsConstraintTranslator::~UnaryDomainBoundsConstraintTranslator() {}

    void UnaryDomainBoundsConstraintTranslator::addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
		auto* con = dynamic_cast<UnaryDomainBoundsConstraint*>(constraint);
		assert( con != nullptr);
		// TODO - Rethink, I think it is not entirely correct... constraint should be placed not on the affected variable but on the relevant variable...
		// besides, these type of cosntraints probably do not make sense in the current CSP model?
// 		Helper::addBoundsConstraintFromDomain(csp, translator, con->getAffected()); 
    }

    BinaryDomainBoundsConstraintTranslator::~BinaryDomainBoundsConstraintTranslator() {

    }

    void BinaryDomainBoundsConstraintTranslator::addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
      auto* con = dynamic_cast<BinaryDomainBoundsConstraint*>(constraint);
      assert( con != nullptr);
//       Helper::addBoundsConstraintFromDomain(csp, translator, con->getAffected()); // TODO - SAME HERE
    }


  }

}
