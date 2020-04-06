
#pragma once

#include <fs/core/base.hxx>

#include <boost/container/flat_set.hpp>
#include <boost/functional/hash.hpp>

#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <unordered_set>
#include <vector>


//! A handy macro for explicitly declaring a variable is not used and avoiding the corresponding warnings (see e.g. http://stackoverflow.com/q/777261)
#define _unused(x) do { (void)sizeof(x);} while (0)

#define WORK_IN_PROGRESS(x) do { assert(false && x); throw std::runtime_error(std::string("UNIMPLEMENTED: ") + x); } while (0)

//! Abort macro
#define FS_ABORT(msg) \
( \
    (std::cerr << "Critical error in file " << __FILE__ \
               << ", line " << __LINE__ << ": " << std::endl \
               << (msg) << std::endl), \
    (abort()), \
    (void)0 \
)

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
	
	// TODO Temporal workaround, this should go to the bin, but it's good to quickly grep the places it's being used
	const TypeIdx UNSPECIFIED_NUMERIC_TYPE = std::numeric_limits<unsigned>::max()-1;
	
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
	
	enum class MetricType { MINIMIZE = 0, MAXIMIZE = 1 };
	
	using FeatureIdx = unsigned;
	const FeatureIdx INVALID_FEATURE = std::numeric_limits<unsigned>::max();

	using TransitionT = std::pair<object_id, object_id>;
	using TransitionGraphT = std::unordered_set<TransitionT,
			boost::hash<TransitionT>>;
	using AllTransitionGraphsT = std::vector<TransitionGraphT>;
	
	
	/**
	 * Custom exceptions
	 */
	class UnimplementedFeatureException : public std::runtime_error {
	public:
		explicit UnimplementedFeatureException(const std::string& msg) : std::runtime_error(msg) {}
	};
	
	class UndefinedValueAccess : public std::runtime_error {
	public:
		explicit UndefinedValueAccess(const std::string& msg) : std::runtime_error(msg) {}
		UndefinedValueAccess() : UndefinedValueAccess("") {}
	};

	class UnindexedAtom : public std::runtime_error {
	public:
		UnindexedAtom(VariableIdx variable, const object_id& value);
	};

} // namespaces
