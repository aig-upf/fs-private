
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
	
	/**
	 * The type of filtering offered by a particular constraint class.
	 *   - Unary: unary constraints filter directly the domain.
	 *   - ArcReduction: Binary constraints arc-reduce their domains wrt one of the two variables of their scope. This is used in the AC3 algorithm.
	 *   - Custom: Other types of constraints might apply any other filtering to ensure some type of local consistency.
	 */
	enum class Filtering {Unary, ArcReduction, Custom};
	
// 	typedef ScopedConstraint const * cptr;
	typedef ScopedConstraint* cptr;
	typedef std::vector<ScopedConstraint::cptr> vcptr;
	
	ScopedConstraint(const VariableIdxVector& scope);
	
	virtual ~ScopedConstraint() {}
	
	virtual Filtering filteringType() = 0;
	
	inline const VariableIdxVector& getScope() const { return _scope; }
	
	inline unsigned getArity() const { return _scope.size(); }
	
	//! Returns true iff the current constraint is satisfied in the given state.
	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
	virtual bool isSatisfied(const State& s) const {
		return this->isSatisfied(Projections::project(s, _scope));
	}
	
	//! Some specialized methods for better performance
	virtual bool isSatisfied(ObjectIdx o) const { throw std::runtime_error("This type of constraint does not support the unary isSatisfied method"); }
	virtual bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const { throw std::runtime_error("This type of constraint does not support the binary isSatisfied method"); }

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

class ParametrizedScopedConstraint : public ScopedConstraint
{
protected:
	//! A vector of arbitrary parameters of the constraint
	const std::vector<int> _binding;
	
public:
	ParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~ParametrizedScopedConstraint() {}
	
	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
};

class UnaryParametrizedScopedConstraint : public ParametrizedScopedConstraint
{
public:
	UnaryParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~UnaryParametrizedScopedConstraint() {};
	
	virtual Filtering filteringType() { return Filtering::Unary; };
	
	bool isSatisfied(const ObjectIdxVector& values) const;
	
	bool isSatisfied(const State& s) const {
		return this->isSatisfied(s.getValue(_scope[0]));
	}
	
	//! To be overriden by the concrete constraint class.
	virtual bool isSatisfied(ObjectIdx o) const = 0;
	
	//! Filters from a new set of domains.
	virtual Output filter(const DomainMap& domains) const;
};


class BinaryParametrizedScopedConstraint : public ParametrizedScopedConstraint
{
public:
	BinaryParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~BinaryParametrizedScopedConstraint() {};

	virtual Filtering filteringType() { return Filtering::ArcReduction; };
	
	bool isSatisfied(const ObjectIdxVector& values) const;
	
	bool isSatisfied(const State& s) const {
		return this->isSatisfied(s.getValue(_scope[0]), s.getValue(_scope[1]));
	}
	
	//! To be overriden by the concrete constraint class.
	virtual bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const = 0;
	
	virtual Output filter(unsigned variable);
};

class GEQConstraint : public BinaryParametrizedScopedConstraint
{
public:
	GEQConstraint(const VariableIdxVector& scope) :
		BinaryParametrizedScopedConstraint(scope, {}) {}
	virtual ~GEQConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
		return o1 >= o2;
	}
};

} // namespaces

