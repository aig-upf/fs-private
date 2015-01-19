
#pragma once

#include <vector>
#include <map>
#include <boost/container/flat_set.hpp>

#include <fs0_types.hxx>

namespace fs0 {

/**
 * A simple class representing an implicit CSP constraint of some type coupled with a custom propagator.
 */
class Constraint
{
protected:
	//! The number of variables of the constraint, cached for performance reasons.
	const unsigned _arity;
	
public:
	typedef std::shared_ptr<Constraint> cptr;
	
	enum class Output {Failure, Pruned, Unpruned};
	
	Constraint(unsigned arity) :
		_arity(arity)
	{
		assert(_arity > 0);
	}
	
	virtual ~Constraint() {}
	
	unsigned getArity() const { return _arity; }
	
	//! Filter (in-place) the given domains to remove inconsistent values.
	virtual Output filter(const DomainVector& domains) = 0;
	
	//!! Return true iff the given values satisfy the constraint.
	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
};

} // namespaces

