
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <utils/projections.hxx>

namespace fs0 {

class ScopedConstraint
{
protected:
	//! The state variables that make up the scope of the constraint.
	const VariableIdxVector _scope;
	
	//! The currently cached projection
	DomainVector projection;
	
public:
	//! The possible results of the filtering process
	enum class Output {Failure, Pruned, Unpruned};
	
// 	typedef ScopedConstraint const * cptr;
	typedef ScopedConstraint* cptr;
	typedef std::vector<ScopedConstraint::cptr> vcptr;
	
	ScopedConstraint(const VariableIdxVector& scope);
	
	virtual ~ScopedConstraint() {}
	
	inline const VariableIdxVector& getScope() const { return _scope; }
	
	inline unsigned getArity() const { return _scope.size(); }
	
	//! Returns true iff the current constraint is satisfied in the given state.
	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
	virtual bool isSatisfied(const State& s) const {
		return this->isSatisfied(Projections::project(s, _scope));
	}

	//! Filters from a new set of domains.
	virtual Output filter(const DomainMap& domains) const  {
		throw std::runtime_error("This type of constraint does not support on-the-fly filtering");
	}
	
	//! Arc-reduces the given variable with respect to the set of currently loaded projections - works only for binary constraints
	virtual Output filter(unsigned variable) {
		throw std::runtime_error("This type of constraint does not support arc-reduction");
	}
	
	//! Filters from the set of currently loaded projections - intended for custom and built-in constraints
	virtual Output filter() {
		throw std::runtime_error("This type of constraint does not support pre-loaded filtering");
	}	
	
	//! Loads (i.e. caches a pointer of) the domain projections of the given state
	void loadDomains(const DomainMap& domains) {
		projection = Projections::project(domains, _scope);
	}
	
	//! Empties the domain cache
	void emptyDomains() { projection.clear(); }
};

class ExternalScopedConstraint : public ScopedConstraint
{
protected:
	//! A vector of arbitrary parameters of the constraint
	const std::vector<int> _binding;
	
public:
	ExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~ExternalScopedConstraint() {}
	
	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
};

class UnaryExternalScopedConstraint : public ExternalScopedConstraint
{
public:
	UnaryExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~UnaryExternalScopedConstraint() {};
	
	//! TODO - This should get proxied to the vector-less version
	bool isSatisfied(const ObjectIdxVector& values) const = 0;
	
	//! To be overriden by the concrete constraint class.
// 	virtual bool isSatisfied(ObjectIdx value) const = 0;

	//! Filters from a new set of domains.
	virtual Output filter(const DomainMap& domains) const;
};


class BinaryExternalScopedConstraint : public ExternalScopedConstraint
{
public:
	BinaryExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~BinaryExternalScopedConstraint() {};
	
	virtual Output filter(unsigned variable);
};

} // namespaces

