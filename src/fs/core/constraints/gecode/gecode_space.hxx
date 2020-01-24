
#pragma once

#include <memory>
#include <gecode/int.hh>


namespace fs0::gecode {

class MinHMaxValueSelector;

/**
 * A Gecode CSP with sets of integer and binary variables, and the possibility of using
 * a custom value selection strategy.
 */
class GecodeSpace : public Gecode::Space {
public:
	GecodeSpace();
	GecodeSpace(GecodeSpace&&) = delete;
	GecodeSpace& operator=(const GecodeSpace&) = delete;
	GecodeSpace& operator=(GecodeSpace&&) = default;
	~GecodeSpace() override;
	
	//! Cloning constructor, required by Gecode
	GecodeSpace(GecodeSpace& other);

	///! Shallow copy operator, mandatory according to the documentation
	virtual Gecode::Space* copy();

	//! Enforce constraint propagation on the CSP; return false if the resulting CSP is
	//! "failed", i.e. not consistent, and return true otherwise
	bool propagate();

	//! Prints a representation of a CSP. Mostly for debugging purposes
	friend std::ostream& operator<<(std::ostream &os, const GecodeSpace&  csp) { return csp.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	void init_value_selector(std::shared_ptr<MinHMaxValueSelector> value_selector);

	int select_value(Gecode::IntVar& x, int csp_var_idx) const;

	//! CSP variables that correspond to the planning problem state variables that are relevant to the goal formula + state constraints
	Gecode::IntVarArray _intvars;
	Gecode::BoolVarArray _boolvars;
	
protected:
	//! A value selector for the branching strategy
	std::shared_ptr<MinHMaxValueSelector> _value_selector;
};

} // namespaces
