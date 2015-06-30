
#pragma once

#include <gecode/int.hh>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <cassert>
#include <vector>
#include <actions.hxx>
#include <atoms.hxx>

namespace fs0 { namespace gecode {

class ActionCSP : public Gecode::Space {
public:
	typedef   ActionCSP *                          ptr;
	typedef   std::vector< ActionCSP::ptr >        vptr;

	ActionCSP( ) {};
	virtual ~ActionCSP() {}

	//! Retrieves possible valuations for a given variable varName
// 	void retrieveAtomsForAffectedVariable( VariableIdx varName, Atom::vctr& atoms ) const;

	//! Retrieve possible ranges of values for a possible variable varName
// 	void retrieveRangesForAffectedVariable( VariableIdx varName, Atom::vrange& ranges  ) const;

	//! Cloning constructor, required by Gecode
	ActionCSP( bool share, ActionCSP& other );

	//! Shallow copy operator, see notes on search in Gecode to
	//! get an idea of what is being "actually" copied
	virtual Gecode::Space* copy( bool share );

	//! Standard copy constructor
	explicit ActionCSP(ActionCSP& other);

	bool checkConsistency();

	//! Prints a representation of a CSP. Mostly for debugging purposes
	friend std::ostream& operator<<(std::ostream &os, const ActionCSP&  csp) { return csp.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << _X;
		return os;
	}

// protected:
	//! CSP variables that correspond to the planning problem state variables that are relevant to the action preconditions and effects
	Gecode::IntVarArray _X;

	//! CSP variables that correspond to the planning problem state variables that are affected by the action's effects
	Gecode::IntVarArray _Y;
};


} } // namespaces
