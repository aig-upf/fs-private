
#pragma once

#include <memory>

namespace fs0 { class TupleIndex; }
namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode { class FormulaCSP; }}

namespace fs0 {

class State;

//! A base interface for a formula satisfiability manager
class FormulaInterpreter {
public:
	//! Factory method - return a formula satisfiability manager appropriate to the given formula
	static FormulaInterpreter* create(const fs::Formula* formula, const TupleIndex& tuple_index);
	
	virtual ~FormulaInterpreter() {}

	//! Returns true if the formula represented by the current object is satisfied in the given state
	virtual bool satisfied(const State& state) const = 0;
};


//! A satisfiability manager that models formula satisfaction as a CSP in order to determine whether a given formula is satisfiable or not.
class DirectFormulaInterpreter : public FormulaInterpreter {
public:
	DirectFormulaInterpreter(const fs::Formula* formula);
	~DirectFormulaInterpreter() = default;

	//! Returns true if the formula represented by the current object is satisfied in the given state
	bool satisfied(const State& state) const;

protected:
	//! The formula whose satisfiability will be directly checked
	const fs::Formula* _formula;
};


//! A satisfiability manager that models formula satisfaction as a CSP in order to determine whether a given formula is satisfiable or not.
class CSPFormulaInterpreter : public FormulaInterpreter {
public:
	CSPFormulaInterpreter(const fs::Formula* formula, const TupleIndex& tuple_index);
	~CSPFormulaInterpreter();

	//! Returns true if the formula represented by the current object is satisfied in the given state
	bool satisfied(const State& state) const;

protected:
	//! The formula handler that will check for CSP applicability
	const gecode::FormulaCSP* _formula_csp;
};

} // namespaces

