
#pragma once

#include <constraints/scoped_constraint.hxx>
#include <constraints/scoped_effect.hxx>
#include <complex_action_manager.hxx>
#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

class ConstraintTranslator  {
public:
	typedef ConstraintTranslator* ptr;

	virtual ~ConstraintTranslator();

	virtual void addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const = 0;
};

class EffectTranslator  {
public:
	typedef EffectTranslator* ptr;

	virtual ~EffectTranslator();

	virtual void addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const = 0;
};


} } // namespaces

