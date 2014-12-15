
#pragma once

#include <vector>
#include <map>
#include <boost/container/flat_set.hpp>

#include <core_types.hxx>

namespace aptk { namespace core {

/**
 * A simple class representing an implicit CSP constraint of some type coupled with a custom propagator.
 */
class Constraint
{
protected:
	//! The constraint variables.
	const VariableIdxVector _variables;
	
	//! The number of variables of the constraint, cached for performance reasons.
	const unsigned num_variables;
	
public:
	typedef std::shared_ptr<Constraint> cptr;
	
	enum class Output {Failure, Pruned, Unpruned};
	
	Constraint(const VariableIdxVector& variables) :
		_variables(variables),
		num_variables(variables.size())
	{}
	
	virtual ~Constraint() {}
	
	const VariableIdxVector& getScope() { return _variables; }
	
	virtual Output enforce_consistency(DomainSet& domains) = 0;
	
	//!! Return true iff the given values satisfy the constraint.
	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
};

} } // namespaces

