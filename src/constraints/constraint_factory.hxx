
#pragma once

#include <constraints/direct/constraint.hxx>

namespace fs0 {

/**
 * Constraint instantiation routines.
 */
class ConstraintFactory
{
public:
	//! Instantiates a constraint of the class corresponding to the given name and with the given scope
	//! The ownership of the returned pointer belongs to the caller
	static const DirectConstraint::cptr create(const std::string& name, const ObjectIdxVector& parameters, const VariableIdxVector& scope);
	
};


} // namespaces

