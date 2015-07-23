
#pragma once

#include <memory>
#include <string>
#include <constraints/scoped_constraint.hxx>


namespace fs0 { class Term; }

namespace fs0 { namespace gecode {


//! Constraint instantiation routines.
class GecodeConstraintFactory {
public:
	//! Instantiates a constraint of the class corresponding to the given name and with the given scope
	//! The ownership of the returned pointer belongs to the caller
	static const ScopedConstraint::cptr create(const Term& term);
	
};


} } // namespaces

