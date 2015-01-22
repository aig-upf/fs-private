
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

/**
 * An action manager operating on a delete-free relaxation of the problem.
 */
class RelaxedApplicabilityManager
{
protected:
	//! The state constraints
	const ScopedConstraint::vcptr& _constraints;

public:
	RelaxedApplicabilityManager(const ScopedConstraint::vcptr& constraints)
		: _constraints(constraints) {}

	//!
	std::pair<bool, FactSetPtr> isApplicable(const Action& action, const State& seed, const DomainMap& domains) const;
	
protected:
	bool isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, FactSetPtr causes) const;
};

} // namespaces
