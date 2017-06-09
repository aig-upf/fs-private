
#pragma once

#include <constraints/filtering.hxx>
#include <constraints/direct/component.hxx>

namespace fs0 {

class ProblemInfo;
class State;

//! A constraint, usually associated with a condition.
class DirectConstraint : public DirectComponent {
protected:
	//! The currently cached projection
	mutable DomainVector projection;
	
public:
	DirectConstraint(const VariableIdxVector& scope);
	DirectConstraint(const VariableIdxVector& scope, const ValueTuple& parameters);
	virtual ~DirectConstraint() = default;

	virtual FilteringType filteringType() const = 0;

	//! Filters from a new set of domains.
	virtual FilteringOutput filter(const DomainMap& domains) const  {
		throw std::runtime_error("This type of constraint does not support on-the-fly filtering");
	}

	//! Arc-reduces the given variable with respect to the set of currently loaded projections - works only for binary constraints
	virtual FilteringOutput filter(unsigned variable) const {
		throw std::runtime_error("This type of constraint does not support arc-reduction");
	}

	//! Filters from the set of currently loaded projections - intended for custom and built-in constraints
	virtual FilteringOutput filter() {
		throw std::runtime_error("This type of constraint does not support pre-loaded filtering");
	}

	//! Loads (i.e. caches a pointer of) the domain projections of the given state
	void loadDomains(const DomainMap& domains) const;

	//! Empties the domain cache
	void emptyDomains() const { projection.clear(); }

	//! Every type of constraint determines whether it makes sense for the constraint to be precompiled or not.
	//! Thus, the compile method must be subclassed and either return NULL, if the constraint shouldn't be compiled,
	//! or an actual compiled constraint, if it should.
	virtual DirectConstraint* compile(const ProblemInfo& problemInfo) const = 0;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const DirectConstraint& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};



class UnaryDirectConstraint : public DirectConstraint {
public:
	UnaryDirectConstraint(const VariableIdxVector& scope, const ValueTuple& parameters);

	~UnaryDirectConstraint() = default;

	virtual FilteringType filteringType() const override { return FilteringType::Unary; };

	//! To be overriden by the concrete constraint class.
	virtual bool isSatisfied(const object_id& o) const = 0;

	//! Filters from a new set of domains.
	virtual FilteringOutput filter(const DomainMap& domains) const override;

	//! All unary constraints are compiled by default
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override;
};


class BinaryDirectConstraint : public DirectConstraint {
public:
	BinaryDirectConstraint(const VariableIdxVector& scope, const ValueTuple& parameters);

	~BinaryDirectConstraint() = default;

	virtual FilteringType filteringType() const override { return FilteringType::ArcReduction; };

	//! To be overriden by the concrete constraint class.
	virtual bool isSatisfied(const object_id& o1, const object_id& o2) const = 0;

	virtual FilteringOutput filter(unsigned variable) const override;

	//! All binary constraints are compiled by default
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override;
};

} // namespaces
