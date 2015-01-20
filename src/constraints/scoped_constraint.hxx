
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <constraints/constraints.hxx>
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
	virtual Constraint::Output filter(const DomainMap& domains) const = 0;
	
	//! Filters from the set of currently loaded projections
	virtual Constraint::Output filter(unsigned variable = std::numeric_limits<unsigned>::max()) {
		throw std::runtime_error("Revise this version of the filter method");
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
	//! The state variables that make up the scope of the constraint.
	const std::vector<int> _parameters;
	
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
	virtual Constraint::Output filter(const DomainMap& domains) const;
};


} // namespaces

