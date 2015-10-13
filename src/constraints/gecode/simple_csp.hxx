
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/utils/value_selection.hxx>
#include <gecode/int.hh>


namespace fs0 { namespace gecode {

/**
 * A SimpleCSP is a Gecode CSP with a single set of integer variables.
 * Should be enough for most of our needs.
 */
class SimpleCSP : public Gecode::Space {
public:
	typedef   SimpleCSP* ptr;

	SimpleCSP() : _value_selector() {};

	//! Cloning constructor, required by Gecode
	SimpleCSP( bool share, SimpleCSP& other ) :
		Gecode::Space(share, other),
		_value_selector(other._value_selector)
	{
		_intvars.update( *this, share, other._intvars );
		_boolvars.update( *this, share, other._boolvars );
	}

	//! Shallow copy operator, see notes on search in Gecode to
	//! get an idea of what is being "actually" copied
	virtual Gecode::Space* copy( bool share ) { return new SimpleCSP( share, *this ); }

	//! Standard copy constructor
	explicit SimpleCSP(SimpleCSP& other);

	bool checkConsistency(){
		Gecode::SpaceStatus st = status();
		return st != Gecode::SpaceStatus::SS_FAILED;
	}

	//! Prints a representation of a CSP. Mostly for debugging purposes
	friend std::ostream& operator<<(std::ostream &os, const SimpleCSP&  csp) { return csp.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << _intvars << std::endl;
		os << _boolvars; 
		return os;
	}
	
	void init(EarliestLayerValueSelector&& value_selector) {
		_value_selector = std::move(value_selector);
	}

	//! CSP variables that correspond to the planning problem state variables that are relevant to the goal formula + state constraints
	Gecode::IntVarArray _intvars;
	Gecode::BoolVarArray _boolvars;
	
	//! A value selector for the branching strategy
	EarliestLayerValueSelector _value_selector;
};

} } // namespaces
