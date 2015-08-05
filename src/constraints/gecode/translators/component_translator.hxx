
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
	virtual void registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const = 0;
	
	//! The translator can register any number of CSP constraints
	virtual void registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const = 0;
};


class ConstantTermTranslator : public TermTranslator {
public:
	ConstantTermTranslator() {}

	void registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const;
	
	void registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
		// Constants produce no particular constraint, since the domain constraint was already posted during creation of the variable
	}
};

class StateVariableTermTranslator : public TermTranslator {
public:
	StateVariableTermTranslator() {}

	void registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const;
	
	void registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
		// State variables produce no particular constraints during registration time
	}
};

class NestedTermTranslator : public TermTranslator {
public:
	NestedTermTranslator() {}

	//! The registration of variables is common to both static- and fluent- headed terms
	void registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const;
};


class StaticNestedTermTranslator : public NestedTermTranslator {
public:
	StaticNestedTermTranslator() {}

	void registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const;
};

class FluentNestedTermTranslator : public NestedTermTranslator {
public:
	FluentNestedTermTranslator() {}

	void registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const;
};



class AtomicFormulaTranslator {
public:
	typedef const AtomicFormulaTranslator* cptr;
	
	AtomicFormulaTranslator() {}
	virtual ~AtomicFormulaTranslator() {}

	//! Most atomic formulae simply need all their subterms to have their variables registered
	virtual void registerVariables(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const;
	
	//! For constraint registration, each particular subclass translator will probably want to add to the common functionality here,
	//! which simply performs the recursive registration of each subterm's own constraints
	virtual void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const;
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

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const;
};

class AlldiffGecodeTranslator : public AtomicFormulaTranslator {
public:
	AlldiffGecodeTranslator() {}

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const;
};

class SumGecodeTranslator : public AtomicFormulaTranslator {
public:
	SumGecodeTranslator() {}

	void registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const;
};

} } // namespaces
