
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <exception>

/**
 * Custom datatypes and typedefs.
 */
namespace aptk  { namespace core {
	
	// A unique identifier for any of the domain actions.
	typedef unsigned ActionIdx;
	
	// A sequence of actions is a plan.
	typedef std::vector<ActionIdx> ActionPlan;
	
	// A unique identifier for any of the domain data types.
	typedef unsigned TypeIdx;
	
	// A unique identifier for any of the problem objects (might be an integer)
	typedef int ObjectIdx;
	
	//! An action signature is a list of (positional) parameters with a given type.
	typedef std::vector<TypeIdx> ActionSignature;
	
	//! The index identifying a state variable.
	typedef unsigned VariableIdx;
	

	typedef std::vector<VariableIdx> VariableIdxVector;
	typedef std::vector<ObjectIdx> ObjectIdxVector;
	typedef ObjectIdxVector ProcedurePoint;
	
} } // namespaces

/**
 * Custom exceptions
 */
namespace aptk  { namespace core {
	
	struct DuplicateObjectException : std::exception {};
	struct DuplicateTypeException   : std::exception {};
	struct DuplicateSymbolException : std::exception {};
	
	struct NonExistingTypeException : std::exception {};
	struct NonImplementedException : std::exception {};
	
} } // namespaces