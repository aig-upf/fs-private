
#pragma once

#include <languages/fstrips/formulae.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

class State;

//! A base interface for a formula satisfiability manager
class FormulaSatisfiabilityManager {
public:
	//! Factory method - return a formula satisfiability manager appropriate to the given formula
	static FormulaSatisfiabilityManager* create(const fs::Formula::cptr formula);
	
	virtual ~FormulaSatisfiabilityManager() {}

	//! Returns true if the formula represented by the current object is satisfied in the given state
	virtual bool satisfied(const State& state) const = 0;
};

} // namespaces

