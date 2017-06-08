
#pragma once

#include <memory>
#include <limits>

#include <vector>
#include <map>
#include <set>
#include <boost/container/flat_set.hpp>

#include <exception>
#include <base.hxx>

//! A handy macro for explicitly declaring a variable is not used and avoiding the corresponding warnings (see e.g. http://stackoverflow.com/q/777261)
#define _unused(x) do { (void)sizeof(x);} while (0)

#define WORK_IN_PROGRESS(x) do { assert(false && x); throw std::runtime_error(std::string("UNIMPLEMENTED: ") + x); } while (0)


/**
 * Custom datatypes and usings.
 */
namespace fs0 {

	// A unique identifier for any of the domain actions.
	using ActionIdx = unsigned;

	// A sequence of actions is a plan.
	using ActionPlan = std::vector<ActionIdx>;

	// A unique identifier for any of the domain data types.
	using TypeIdx = unsigned;
	const TypeIdx INVALID_TYPE = std::numeric_limits<unsigned>::max();
	
	//! The ID of a logical symbol
	using SymbolIdx = unsigned;

	// A unique identifier for any of the problem objects (might be an integer)
// 	using ObjectIdx = int;

	//! An action signature is a list of (positional) parameters with a given type.
	using Signature = std::vector<TypeIdx>;

	//! The index identifying a state variable.
	using VariableIdx = unsigned;
	const VariableIdx INVALID_VARIABLE = std::numeric_limits<unsigned>::max();

	using VariableIdxVector = std::vector<VariableIdx>;

	//! A tuple of values
	using ValueTuple = std::vector<object_id>;
	
	//! The index of a tuple of a FSTRIPS logical symbol, unique across all symbols.
	using AtomIdx = unsigned;
	const AtomIdx INVALID_TUPLE = std::numeric_limits<unsigned int>::max();
	
	using Support = std::vector<AtomIdx>;

	//! A domain is a set of values (of a state variable)
 	using Domain = boost::container::flat_set<object_id>;
	using DomainPtr = std::shared_ptr<Domain>;

	//! A vector of domains.
	using DomainVector = std::vector<DomainPtr>;

	//! A map mapping state variable indexes to their possible domains.
	using DomainMap = std::map<VariableIdx, DomainPtr>;
	
	//! A map mapping a subset of state variables to possible values
	using PartialAssignment = std::map<VariableIdx, object_id>;

	//! A function in the logical sense.
	using Function = std::function<object_id(const ValueTuple&)>;
	
	
	/**
	 * Custom exceptions
	 */
	class UnimplementedFeatureException : public std::runtime_error {
	public:
		UnimplementedFeatureException(const std::string& msg) : std::runtime_error(msg) {}
	};
	
	class InvalidConfiguration : public std::runtime_error {
	public:
		InvalidConfiguration(const std::string& msg) : std::runtime_error(msg) {}
	};
	
	class UndefinedValueAccess : public std::runtime_error {
	public:
		UndefinedValueAccess(const std::string& msg) : std::runtime_error(msg) {}
		UndefinedValueAccess() : UndefinedValueAccess("") {}
	};

} // namespaces
