
#pragma once

#include <languages/fstrips/language.hxx>
#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

enum class CSPVariableType;

class SimpleCSP; class GecodeCSPVariableTranslator;

class TermTranslator {
public:
	typedef const TermTranslator* cptr;

	TermTranslator() {}
	virtual ~TermTranslator() {}

	//! The translator can optionally register any number of (probably temporary) CSP variables.
	virtual void registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const = 0;

	//! The translator can register any number of CSP constraints
	virtual void registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const = 0;
};


class ConstantTermTranslator : public TermTranslator {
public:
	ConstantTermTranslator() {}

	void registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;

	void registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const {
		// Constants produce no particular constraint, since the domain constraint was already posted during creation of the variable
	}
};

class StateVariableTermTranslator : public TermTranslator {
public:
	StateVariableTermTranslator() {}

	void registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;

	void registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const {
		// State variables produce no particular constraints during registration time
	}
};

class NestedTermTranslator : public TermTranslator {
public:
	NestedTermTranslator() {}

	virtual ~NestedTermTranslator();
	//! The registration of variables is common to both static- and fluent- headed terms
	virtual void registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;

protected:
	//! Do the actual registration on the translator. Can be overriden if a particular logic is necessary
	virtual void do_root_registration(const fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const = 0;
};

class StaticNestedTermTranslator : public NestedTermTranslator {
public:
	StaticNestedTermTranslator() {}

	void do_root_registration(const fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;
	
	void registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;
};

class FluentNestedTermTranslator : public NestedTermTranslator {
public:
	FluentNestedTermTranslator() {}

	void do_root_registration(const fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;
	
	void registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;
};

class ArithmeticTermTranslator : public NestedTermTranslator {
public:
	ArithmeticTermTranslator() {}

	void registerConstraints(const fs::Term::cptr formula, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;

protected:
	// Might want to be overriden by some subclass
	Gecode::IntRelType getRelationType() const { return Gecode::IRT_EQ; }

	virtual Gecode::IntArgs getLinearCoefficients() const = 0;

	virtual void post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const = 0;

	//! Arithmetic terms do a special type of registration for the temporary variable
	void do_root_registration(const fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;
};

class AdditionTermTranslator : public ArithmeticTermTranslator {
public:
	Gecode::IntArgs getLinearCoefficients() const;

	void post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const;
};

class SubtractionTermTranslator : public ArithmeticTermTranslator {
public:
	Gecode::IntArgs getLinearCoefficients() const;

	void post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const;
};

class MultiplicationTermTranslator : public ArithmeticTermTranslator {
public:
	Gecode::IntArgs getLinearCoefficients() const;

	void post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const;
};

class AtomicFormulaTranslator {
public:
	typedef const AtomicFormulaTranslator* cptr;

	AtomicFormulaTranslator() {}
	virtual ~AtomicFormulaTranslator() {}

	//! Most atomic formulae simply need all their subterms to have their variables registered
	virtual void registerVariables(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) const;

	//! For constraint registration, each particular subclass translator will probably want to add to the common functionality here,
	//! which simply performs the recursive registration of each subterm's own constraints
	virtual void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;
};

class RelationalFormulaTranslator : public AtomicFormulaTranslator {
protected:
	//! A helper data structure to help translate language relational symbols to gecode relational operators
	const static std::map<fs::RelationalFormula::Symbol, Gecode::IntRelType> symbol_to_gecode;

	//! A helper function to recover the gecode relational operator from a formula
	static Gecode::IntRelType gecode_symbol(fs::RelationalFormula::cptr formula);

	//! A helper data structure to map gecode operators with their inverse.
	const static std::map<Gecode::IntRelType, Gecode::IntRelType> operator_inversions;

	//! A helper to invert (e.g. from < to >=) a given gecode relational operator
	static Gecode::IntRelType invert_operator(Gecode::IntRelType op);

	//! The actual symbol of the current relational formula
	fs::RelationalFormula::Symbol _symbol;

public:
	RelationalFormulaTranslator() {}

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;
};

class AlldiffGecodeTranslator : public AtomicFormulaTranslator {
public:
	AlldiffGecodeTranslator() {}

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;
};

class SumGecodeTranslator : public AtomicFormulaTranslator {
public:
	SumGecodeTranslator() {}

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;
};

class ExtensionalTranslator : public AtomicFormulaTranslator {
public:
	ExtensionalTranslator(const std::string& symbol) : _symbol(symbol) {}

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator) const;

protected:
	const std::string _symbol;
};

} } // namespaces
