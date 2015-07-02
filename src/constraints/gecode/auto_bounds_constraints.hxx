
#pragma once 

#include <constraints/gecode/expr_translators.hxx>

namespace fs0 { namespace gecode {

class UnaryDomainBoundsConstraintTranslator : public ConstraintTranslator  {
public:
	virtual ~UnaryDomainBoundsConstraintTranslator();
	virtual void addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};


class BinaryDomainBoundsConstraintTranslator : public ConstraintTranslator  {
public:
	virtual ~BinaryDomainBoundsConstraintTranslator();
	virtual void addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};

} } // namespaces

