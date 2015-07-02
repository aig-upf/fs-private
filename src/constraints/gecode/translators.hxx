	
#pragma once


#include <constraints/scoped_constraint.hxx>
#include <constraints/scoped_effect.hxx>

#include <gecode/int.hh>


namespace fs0 { namespace gecode {
	
class GecodeCSPTranslator; class SimpleCSP;

class ConstraintTranslator {
public:
	typedef ConstraintTranslator* ptr;

	virtual ~ConstraintTranslator();

	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const = 0;
};


class UnaryDomainBoundsConstraintTranslator : public ConstraintTranslator  {
public:
	virtual ~UnaryDomainBoundsConstraintTranslator();
	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};


class BinaryDomainBoundsConstraintTranslator : public ConstraintTranslator  {
public:
	virtual ~BinaryDomainBoundsConstraintTranslator();
	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};


class GecodeRelationTranslator : public ConstraintTranslator {
protected:
	Gecode::IntRelType _type;
public:
	GecodeRelationTranslator(Gecode::IntRelType type) : _type(type) {}
};


//! Relation constraints between a variable and a constant
class UnaryRelationTranslator : public GecodeRelationTranslator {
public:
	UnaryRelationTranslator(Gecode::IntRelType type) : GecodeRelationTranslator(type) {}
	
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};

//! Relation constraints between two variables
class BinaryRelationTranslator : public GecodeRelationTranslator {
public:
	BinaryRelationTranslator(Gecode::IntRelType type) : GecodeRelationTranslator(type) {}

	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};


class EffectTranslator {
public:
	typedef EffectTranslator* ptr;

	virtual ~EffectTranslator();

	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const = 0;
};

class ValueAssignmentEffectTranslator : public EffectTranslator  {
public:
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const;
};

class AdditiveUnaryEffectTranslator : public EffectTranslator  {
public:
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const;
};


//! Perform the actual registration of all available translators (to be invoked during bootstrap)
void registerTranslators();

} } // namespaces
