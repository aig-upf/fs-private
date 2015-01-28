
#pragma once

#include <memory>

#include <vector>
#include <map>
#include <set>
#include <boost/container/flat_set.hpp>

#include <exception>

/**
 * Custom datatypes and typedefs.
 */
namespace fs0 {
	
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


	//! A domain is a set of values (of a state variable)
 	typedef boost::container::flat_set<ObjectIdx> Domain;
//  	typedef std::set<ObjectIdx> Domain;
	typedef std::shared_ptr<Domain> DomainPtr;
	
	//! A vector of domains.
	typedef std::vector<DomainPtr> DomainVector;
	
	//! A map mapping state variable indexes to their possible domains.
	typedef std::map<VariableIdx, DomainPtr> DomainMap;
	
	/**
	 * Custom exceptions
	 */
	struct DuplicateObjectException : std::exception {};
	struct DuplicateTypeException   : std::exception {};
	struct DuplicateSymbolException : std::exception {};
	
	struct NonExistingTypeException : std::exception {};
	struct NonImplementedException : std::exception {};
	
	struct InvalidConstraintException : std::exception {};
	
} // namespaces