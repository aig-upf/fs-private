
#pragma once

#include <memory>
#include <string>
#include <actions.hxx>
#include <constraints/problem_constraints.hxx>


namespace fs0 {

/**
 * Constraint instantiation routines.
 */
class ConstraintFactory
{
public:
	//! Instantiates a constraint of the class corresponding to the given name and with the given scope
	//! The ownership of the returned pointer belongs to the caller
	static const ProblemConstraint::cptr create(const std::string& name, const VariableIdxVector& scope);
	
	//! Instantiates a constraint of the class corresponding to the given name and with the given scope
	//! The ownership of the returned pointer belongs to the caller
	static const ProblemConstraint::cptr createExternalConstraint(ApplicableEntity::cptr entity, unsigned procedureIdx);
};


} // namespaces

