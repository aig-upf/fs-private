
#pragma once

#include <memory>
#include <gecode/int.hh>
#include <constraints/gecode/utils/value_selection.hxx>


namespace fs0 { namespace gecode {

/**
 * A GecodeCSP is a Gecode CSP with a single set of integer variables.
 * Should be enough for most of our needs.
 */
class GecodeCSP : public Gecode::Space {
public:
	GecodeCSP();
	GecodeCSP(GecodeCSP&) = delete;
	GecodeCSP(GecodeCSP&&) = default;
	GecodeCSP& operator=(const GecodeCSP&) = delete;
	GecodeCSP& operator=(GecodeCSP&&) = default;
	~GecodeCSP();
	
	//! Cloning constructor, required by Gecode
	GecodeCSP(bool share, GecodeCSP& other);

	//! Shallow copy operator, see notes on search in Gecode to
	//! get an idea of what is being "actually" copied
	virtual Gecode::Space* copy(bool share);

	bool checkConsistency();

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
