
#pragma once

#include <gecode/int.hh>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <cassert>
#include <vector>
#include <actions.hxx>
#include <atoms.hxx>

namespace fs0 { namespace gecode {

/**
 * A SimpleCSP is a Gecode CSP with a single set of integer variables.
 * Should be enough for most of our needs.
 */
class SimpleCSP : public Gecode::Space {
public:
	typedef   SimpleCSP* ptr;

	SimpleCSP( ) {};
// 	virtual ~SimpleCSP() {}

	//! Cloning constructor, required by Gecode
	SimpleCSP( bool share, SimpleCSP& other ) :
		Gecode::Space( share, other ) {
		_X.update( *this, share, other._X );
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
	std::ostream& print(std::ostream& os) const { os << _X; return os; }

// protected:
	//! CSP variables that correspond to the planning problem state variables that are relevant to the goal formula + state constraints
	Gecode::IntVarArray _X;
};


} } // namespaces
