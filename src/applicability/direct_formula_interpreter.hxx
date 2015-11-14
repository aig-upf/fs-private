
#pragma once

#include <applicability/formula_interpreter.hxx>

namespace fs0 {

//! An satisfiability manager that models formula satisfaction as a CSP in order to determine whether a given formula is satisfiable or not.
class DirectFormulaInterpreter : public FormulaInterpreter {
public:
	DirectFormulaInterpreter(const fs::Formula::cptr formula);

	//! Returns true if the formula represented by the current object is satisfied in the given state
	bool satisfied(const State& state) const;

protected:
	//! The formula whose satisfiability will be directly checked
	const fs::Formula::cptr _formula;
};

} // namespaces

