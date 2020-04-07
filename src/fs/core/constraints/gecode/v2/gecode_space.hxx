
#pragma once

#include <gecode/int.hh>

namespace fs0::gecode::v2 {

//! A Gecode CSP with sets of integer and binary variables
class FSGecodeSpace : public Gecode::Space {
public:
    FSGecodeSpace();
	FSGecodeSpace(std::size_t numintvars, std::size_t numboolvars);
    ~FSGecodeSpace() override = default;

	FSGecodeSpace(FSGecodeSpace&&) = delete;
	FSGecodeSpace& operator=(const FSGecodeSpace&) = delete;
	FSGecodeSpace& operator=(FSGecodeSpace&&) = default;

	
	//! Cloning constructor, required by Gecode
	FSGecodeSpace(FSGecodeSpace& other);

	///! Shallow copy operator, mandatory according to the documentation
	Gecode::Space* copy() override;

	//! Force constraint propagation on the underlying CSP and return whether it is consistent
	bool propagate();

	//! Prints a representation of a CSP. Mostly for debugging purposes
	friend std::ostream& operator<<(std::ostream &os, const FSGecodeSpace&  csp) { return csp.print(os); }
	std::ostream& print(std::ostream& os) const;

	//! CSP variables that correspond to the planning problem state variables that are relevant to the goal formula + state constraints
	Gecode::IntVarArray intvars;
	Gecode::BoolVarArray boolvars;

    //! The ID of a CSP variable is made up of a char denoting whether it is an integer ('i') or boolean ('b')
    //! variable, plus an int denoting its position within intvars/boolvars.
    using var_id = std::pair<char, int>;
};

} // namespaces
