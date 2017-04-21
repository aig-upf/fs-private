
#pragma once

#include <memory>
#include <limits>

#include <vector>
#include <map>
#include <set>
#include <boost/container/flat_set.hpp>
#include <boost/variant.hpp>
#include <boost/functional/hash.hpp>

#include <exception>

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
	using ObjectIdx = boost::variant<int32_t,float>;

	//! An action signature is a list of (positional) parameters with a given type.
	using Signature = std::vector<TypeIdx>;

	//! The index identifying a state variable.
	using VariableIdx = unsigned;
	const VariableIdx INVALID_VARIABLE = std::numeric_limits<unsigned>::max();

	using VariableIdxVector = std::vector<VariableIdx>;

	using ObjectIdxVector = std::vector<ObjectIdx>;

	//! A tuple of values
	using ValueTuple = std::vector<ObjectIdx>;

	//! The index of a tuple of a FSTRIPS logical symbol, unique across all symbols.
	using AtomIdx = unsigned;
	const AtomIdx INVALID_TUPLE = std::numeric_limits<unsigned int>::max();

	using Support = std::vector<AtomIdx>;

	//! A domain is a set of values (of a state variable)
 	using Domain = boost::container::flat_set<ObjectIdx>;
	using DomainPtr = std::shared_ptr<Domain>;

	//! A vector of domains.
	using DomainVector = std::vector<DomainPtr>;

	//! A map mapping state variable indexes to their possible domains.
	using DomainMap = std::map<VariableIdx, DomainPtr>;

	//! A map mapping a subset of state variables to possible values
	using PartialAssignment = std::map<VariableIdx, ObjectIdx>;

	//! A function in the logical sense.
	using Function = std::function<ObjectIdx(const ValueTuple&)>;


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

// std specializations for terms and term pointers that will allow us to use them in hash-table-like structures
// NOTE that these specializations are necessary for any use of term pointers in std::map/std::unordered_maps,
// even if compilation will succeed without them as well.

namespace std {
    template<> struct hash<fs0::ObjectIdx> {
        std::size_t operator()(const fs0::ObjectIdx& obj) const {
            return boost::hash<fs0::ObjectIdx>()(obj);
        }
    };
}
