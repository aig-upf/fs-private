
#pragma once

#include <iosfwd>
#include <app_entity.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

/**
 * An action manager operating on a delete-free relaxation of the problem.
 */
class RelaxedActionSetManager
{
protected:
	//! The original state.
	const State* seed;

	//! The state constraints
	const ScopedConstraint::vcptr& _constraints;

public:
	RelaxedActionSetManager(const ScopedConstraint::vcptr& constraints)
		: RelaxedActionSetManager(NULL, constraints) {}
	
	RelaxedActionSetManager(const State* originalState, const ScopedConstraint::vcptr& constraints)
		: seed(originalState), _constraints(constraints) {}

	RelaxedActionSetManager(const RelaxedActionSetManager& other)
		: seed(other.seed), _constraints(other._constraints)  {}
	
	//!
	std::pair<bool, FactSetPtr> isApplicable(const Action& action, const DomainMap& domains) const;
	
	//!
	void computeChangeset(const Action& action, const DomainMap& domains, Changeset& changeset) const;
	
protected:
	bool isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, FactSetPtr causes) const;
	
	void computeProcedurePointChangeset(const ScopedEffect::cptr effect, const VariableIdxVector& relevant, const ObjectIdxVector& values, Changeset& changeset) const;
};

} // namespaces
