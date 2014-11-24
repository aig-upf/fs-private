
#pragma once

#include <vector>
#include <map>
#include <boost/container/flat_set.hpp>

#include <core_types.hxx>

namespace aptk { namespace core {

//! A DomainSet maps variables indexes to an array with all the possible values for that variable.
typedef boost::container::flat_set<ObjectIdx> DomainSetVector;
typedef boost::container::flat_set<int> IntDomainSetVector;
typedef std::map<unsigned, DomainSetVector> DomainSet;
typedef std::vector<DomainSetVector*> ULightDomainSet;
typedef std::vector<IntDomainSetVector*> LightDomainSet;

	
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
};

} } // namespaces

