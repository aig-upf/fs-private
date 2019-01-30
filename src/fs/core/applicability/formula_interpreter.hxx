
#pragma once

#include <fs/core/languages/fstrips/language_fwd.hxx>

#include <memory>

namespace fs0 {

class State;
class AtomIndex;

//! A base interface for a formula satisfiability manager
class FormulaInterpreter {
public:
	//! Factory method - return a formula satisfiability manager appropriate to the given formula
	static FormulaInterpreter* create(const fs::Formula* formula, const AtomIndex& tuple_index);
	
	FormulaInterpreter(const fs::Formula* formula);
	virtual ~FormulaInterpreter();
	FormulaInterpreter(const FormulaInterpreter&);
	
	//! Clone idiom
	virtual FormulaInterpreter* clone() const = 0;

	//! Returns true if the formula represented by the current object is satisfied in the given state
	virtual bool satisfied(const State& state) const = 0;

protected:
	//! The formula whose satisfiability will be directly checked
	const fs::Formula* _formula;
};


//! A satisfiability manager that models formula satisfaction as a CSP in order to determine whether a given formula is satisfiable or not.
class DirectFormulaInterpreter : public FormulaInterpreter {
public:
	DirectFormulaInterpreter(const fs::Formula* formula) : FormulaInterpreter(formula) {}
	~DirectFormulaInterpreter() = default;
	DirectFormulaInterpreter(const DirectFormulaInterpreter&) = default;

	DirectFormulaInterpreter* clone() const { return new DirectFormulaInterpreter(*this); }
	
	//! Returns true if the formula represented by the current object is satisfied in the given state
	bool satisfied(const State& state) const;
};


} // namespaces

