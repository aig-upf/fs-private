
#pragma once

#include <map>

#include <gecode/int.hh>

namespace fs0 { namespace language { namespace fstrips { class Term; class Formula; class AtomicFormula; class RelationalFormula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class GecodeCSP; class CSPTranslator;

class TermTranslator {
public:
	typedef const TermTranslator* cptr;

	TermTranslator() {}
	virtual ~TermTranslator() {}

	//! The translator can optionally register any number of (probably temporary) CSP variables.
	virtual void registerVariables(const fs::Term* term, CSPTranslator& translator) const = 0;

	//! The translator can register any number of CSP constraints
	virtual void registerConstraints(const fs::Term* term, CSPTranslator& translator) const = 0;
};


class ConstantTermTranslator : public TermTranslator {
public:
	void registerVariables(const fs::Term* term, CSPTranslator& translator) const;

	// Constants produce no particular constraint, since the domain constraint was already posted during creation of the variable
	void registerConstraints(const fs::Term* term, CSPTranslator& translator) const {}
};

class BoundVariableTermTranslator : public TermTranslator {
public:
	void registerVariables(const fs::Term* term, CSPTranslator& translator) const;

	// Constants produce no particular constraint, since the domain constraint was already posted during creation of the variable
	void registerConstraints(const fs::Term* term, CSPTranslator& translator) const {}
};

class StaticNestedTermTranslator : public TermTranslator {
public:
	void registerVariables(const fs::Term* term, CSPTranslator& translator) const;
	
	void registerConstraints(const fs::Term* term, CSPTranslator& translator) const;
};

class ArithmeticTermTranslator : public TermTranslator {
public:
	void registerVariables(const fs::Term* term, CSPTranslator& translator) const;

	void registerConstraints(const fs::Term* formula, CSPTranslator& translator) const;

protected:
	// Might want to be overriden by some subclass
	Gecode::IntRelType getRelationType() const { return Gecode::IRT_EQ; }

	virtual Gecode::IntArgs getLinearCoefficients() const = 0;

	virtual void post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const = 0;
};

class AdditionTermTranslator : public ArithmeticTermTranslator {
public:
	Gecode::IntArgs getLinearCoefficients() const;

	void post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const;
};

class SubtractionTermTranslator : public ArithmeticTermTranslator {
public:
	Gecode::IntArgs getLinearCoefficients() const;

	void post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const;
};

class MultiplicationTermTranslator : public ArithmeticTermTranslator {
public:
	Gecode::IntArgs getLinearCoefficients() const;

	void post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const;
};

class FormulaTranslator {
public:
	typedef const FormulaTranslator* cptr;

	FormulaTranslator() {}
	virtual ~FormulaTranslator() {}

	//! Most atomic formulae simply need all their subterms to have their variables registered
	virtual void registerVariables(const fs::AtomicFormula* formula, CSPTranslator& translator) const {}

	//! For constraint registration, each particular subclass translator will probably want to add to the common functionality here,
	//! which simply performs the recursive registration of each subterm's own constraints
	virtual void registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const {}
};

class ExistentiallyQuantifiedFormulaTranslator : public FormulaTranslator {
public:
	// The translator for existentially quantified formulas doesn't need to do anything,
	// since the CSP variables will get registered by the BoundVariable translator
};

class ConjunctionTranslator : public FormulaTranslator {
public:
	// ATM the translator for conjunctions does not need to do anything, since all atomic formulas are extracted anyway and their variables
	// and constraints translated.
	// If we ever want to implement arbitrary boolean formulas, this will need to change.
};

class RelationalFormulaTranslator : public FormulaTranslator {
protected:
	//! A helper function to recover the gecode relational operator from a formula
	static Gecode::IntRelType gecode_symbol(const fs::RelationalFormula* formula);

	//! A helper data structure to map gecode operators with their inverse.
	const static std::map<Gecode::IntRelType, Gecode::IntRelType> operator_inversions;

	//! A helper to invert (e.g. from < to >=) a given gecode relational operator
	static Gecode::IntRelType invert_operator(Gecode::IntRelType op);

public:
	RelationalFormulaTranslator() {}

	void registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const;
};

class AlldiffGecodeTranslator : public FormulaTranslator {
public:
	AlldiffGecodeTranslator() {}

	void registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const;
};

class SumGecodeTranslator : public FormulaTranslator {
public:
	SumGecodeTranslator() {}

	void registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const;
};

//! A Gecode translator that converts whatever arbitrary formula into an equivalent extensional constraint.
class ExtensionalTranslator : public FormulaTranslator {
public:
	ExtensionalTranslator() {}

	void registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const;
};

} } // namespaces
