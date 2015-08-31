
#pragma once

#include <fs0_types.hxx>
#include <constraints/filtering.hxx>
#include <constraints/direct/component.hxx>

namespace fs0 {

class ProblemInfo; class State;

//! A constraint, usually associated with a condition.
class DirectConstraint : public DirectComponent {
protected:
	//! The currently cached projection
	DomainVector projection;
	
public:
	typedef DirectConstraint* cptr;
	typedef std::vector<DirectConstraint::cptr> vcptr;
	
	DirectConstraint(const VariableIdxVector& scope);
	DirectConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	virtual ~DirectConstraint() {}

	virtual FilteringType filteringType() = 0;

	//! Filters from a new set of domains.
	virtual FilteringOutput filter(const DomainMap& domains) const  {
		throw std::runtime_error("This type of constraint does not support on-the-fly filtering");
	}

	//! Arc-reduces the given variable with respect to the set of currently loaded projections - works only for binary constraints
	virtual FilteringOutput filter(unsigned variable) {
		throw std::runtime_error("This type of constraint does not support arc-reduction");
	}

	//! Filters from the set of currently loaded projections - intended for custom and built-in constraints
	virtual FilteringOutput filter() {
		throw std::runtime_error("This type of constraint does not support pre-loaded filtering");
	}

	//! Loads (i.e. caches a pointer of) the domain projections of the given state
	void loadDomains(const DomainMap& domains);

	//! Empties the domain cache
	void emptyDomains() { projection.clear(); }

	//! Every type of constraint determines whether it makes sense for the constraint to be precompiled or not.
	//! Thus, the compile method must be subclassed and either return NULL, if the constraint shouldn't be compiled,
	//! or an actual compiled constraint, if it should.
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const = 0;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const DirectConstraint& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const {
		os << "<unnamed constraint>";
		return os;
	}
};



class UnaryDirectConstraint : public DirectConstraint {
public:
	UnaryDirectConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);

	virtual ~UnaryDirectConstraint() {};

	virtual FilteringType filteringType() { return FilteringType::Unary; };

	//! To be overriden by the concrete constraint class.
	virtual bool isSatisfied(ObjectIdx o) const = 0;

	//! Filters from a new set of domains.
	virtual FilteringOutput filter(const DomainMap& domains) const;

	//! All unary constraints are compiled by default
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const;
};


class BinaryDirectConstraint : public DirectConstraint {
public:
	BinaryDirectConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);

	virtual ~BinaryDirectConstraint() {};

	virtual FilteringType filteringType() { return FilteringType::ArcReduction; };

	//! To be overriden by the concrete constraint class.
	virtual bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const = 0;

	virtual FilteringOutput filter(unsigned variable);

	//! All binary constraints are compiled by default
	virtual DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const;
};

} // namespaces
