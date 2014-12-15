
#pragma once

#include <memory>

#include <vector>
#include <map>
#include <unordered_map>
#include <boost/container/flat_set.hpp>

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
	
	//! A DomainSet maps variables indexes to an array with all the possible values for that variable.
	typedef boost::container::flat_set<ObjectIdx> DomainSetVector;
	typedef std::map<unsigned, DomainSetVector> DomainSet;
	typedef std::vector<DomainSetVector*> LightDomainSet;
	
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
	
	struct InvalidConstraintException : std::exception {};
	
} } // namespaces