
#pragma once

#include <memory>
#include <gecode/int.hh>


namespace fs0 { namespace gecode {

class MinHMaxValueSelector;

/**
 * A Gecode CSP with sets of integer and binary variables, and the possibility of using
 * a custom value selection strategy.
 */
class GecodeCSP : public Gecode::Space {
public:
	GecodeCSP();
	GecodeCSP(GecodeCSP&) = delete;
	GecodeCSP(GecodeCSP&&) = default;
	GecodeCSP& operator=(const GecodeCSP&) = delete;
	GecodeCSP& operator=(GecodeCSP&&) = default;
	~GecodeCSP(); // We define the destructor in the cxx file so that there's no need to include
	              // the header for MinHMaxValueSelector here
	
	//! Cloning constructor, required by Gecode
	GecodeCSP(bool share, GecodeCSP& other);

	//! Shallow copy operator, see notes on search in Gecode to
	//! get an idea of what is being "actually" copied
	virtual Gecode::Space* copy(bool share);

	//! Enforce constraint propagation on the CSP; return false if the resulting CSP is
	//! "failed", i.e. not consistent, and return true otherwise
	bool propagate();

	//! Prints a representation of a CSP. Mostly for debugging purposes
	friend std::ostream& operator<<(std::ostream &os, const GecodeCSP&  csp) { return csp.print(os); }
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

} } // namespaces
