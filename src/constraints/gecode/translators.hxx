	
#pragma once


#include <constraints/scoped_constraint.hxx>
#include <constraints/scoped_effect.hxx>

#include <gecode/int.hh>


namespace fs0 { namespace gecode {
	
class GecodeCSPTranslator; class SimpleCSP;


//! Abstract constraint translator
class ConstraintTranslator {
public:
	typedef ConstraintTranslator* ptr;

	virtual ~ConstraintTranslator();

	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const = 0;
};

//! Abstract effect translator
class EffectTranslator {
public:
	typedef EffectTranslator* ptr;

	virtual ~EffectTranslator();

	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const = 0;
};


class ExtensionalUnaryConstraintTranslator : public ConstraintTranslator  {
public:
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};

class ExtensionalBinaryConstraintTranslator : public ConstraintTranslator  {
public:
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
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

//! An abstract class for constraints that can be modeled as Gecode relations.
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

//! A translator for the 'sum' global constraint
class SumConstraintTranslator : public ConstraintTranslator  {
public:
	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};

//! A translator for the 'alldiff' global constraint
class AlldiffConstraintTranslator : public ConstraintTranslator  {
public:
	virtual void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const;
};


class ExtensionalUnaryEffectTranslator : public EffectTranslator  {
public:
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr constraint) const;
};

class ValueAssignmentEffectTranslator : public EffectTranslator  {
public:
	void addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const;
};


class VariableAssignmentEffectTranslator : public EffectTranslator  {
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
