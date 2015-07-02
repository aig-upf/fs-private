
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <utils/projections.hxx>

namespace fs0 {

class ProblemInfo; class UnaryScopedEffect; class BinaryScopedEffect;

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

	//! Every type of constraint determines whether it makes sense for the constraint to be precompiled or not.
	//! Thus, the compile method must be subclassed and either return NULL, if the constraint shouldn't be compiled,
	//! or an actual compiled constraint, if it should.
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const = 0;

	virtual std::string getName() const {return std::string("<unnamed constraint>");}
};

class ParametrizedScopedConstraint : public ScopedConstraint
{
protected:
	//! A vector of arbitrary parameters of the constraint
	const std::vector<int> _binding;

public:
	ParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);

	virtual ~ParametrizedScopedConstraint() {}

	const std::vector<int>& getParameters() const { return _binding; }

	virtual bool isSatisfied(const ObjectIdxVector& values) const = 0;
	
	const std::vector<int>& getBinding() const { return _binding; }
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

	//! All unary constraints are compiled by default
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const;
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

	//! All binary constraints are compiled by default
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const;
};


/**
 * A UnaryDomainBoundsConstraint applies to a relevant variable X and checks that
 * the application of a certain action effect upon it produces a value for another variable Y
 * which does not violate the allowed bounds for Y.
 *
 * As an example, if X is of type int[0..10] and we have an action effect X := f(Y), we will want to
 * place a UnaryDomainBoundsConstraint upon Y which checks that 0 <= f(Y) <= 10.
 */
class UnaryDomainBoundsConstraint : public UnaryParametrizedScopedConstraint {
protected:
	const ProblemInfo& _problemInfo;

	const UnaryScopedEffect* _effect;

public:
	UnaryDomainBoundsConstraint(const UnaryScopedEffect* effect, const ProblemInfo& problemInfo);

	virtual ~UnaryDomainBoundsConstraint() {};

	bool isSatisfied(ObjectIdx o) const;

	VariableIdx getAffected() const;
};

/**
 * A BinaryScopedEffect is like a UnaryDomainBoundsConstraint but works for arity-2 effects.
 *
 * An example would be an effect X := X - Y, where we will want to place a binary constraint
 * that makes sure that X-Y remains within the bounds of the variable X.
 */
class BinaryDomainBoundsConstraint : public BinaryParametrizedScopedConstraint {
protected:
	const ProblemInfo& _problemInfo;

	const BinaryScopedEffect* _effect;

public:
	BinaryDomainBoundsConstraint(const BinaryScopedEffect* effect, const ProblemInfo& problemInfo);

	virtual ~BinaryDomainBoundsConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const;

	VariableIdx getAffected() const;
};
} // namespaces
