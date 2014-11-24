
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <exception>

/**
 * Useful datatypes and typedefs.
 */
namespace aptk  { namespace core {
	
	// A unique identifier for any of the domain actions.
	typedef unsigned ActionIdx;
	typedef unsigned BoundActionIdx;
	
	// A sequence of actions is a plan.
	typedef std::vector<BoundActionIdx> ActionPlan;
	
	// A unique identifier for any of the domain data types.
	typedef unsigned TypeIdx;
	
	// A unique identifier for any of the problem objects.
	typedef unsigned ObjectIdx;
	
	/**
	 * An action signature is a list of (positional) parameters with a given type.
	 */
	typedef std::vector<TypeIdx> ActionSignature;
	
	/**
	 * A parameter binding is a list mapping parameter positions with actual object IDs
	 * (which should be of the type declared by the positional parameter).
	 */
	typedef std::vector<ObjectIdx> ParameterBinding;
	
	//! The index identifying a state variable.
	typedef unsigned VariableIdx;
	typedef std::vector<VariableIdx> VariableIdxVector;
	typedef std::vector<ObjectIdx> ObjectIdxVector;
	
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