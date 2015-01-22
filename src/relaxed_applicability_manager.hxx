
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

/**
 * An applicability manager operating on a delete-free relaxation of the problem.
 * Determines the applicability of actions disregarding state constraints.
 */
class RelaxedApplicabilityManager
{
public:
	static RelaxedApplicabilityManager* createApplicabilityManager(const Action::vcptr& actions);
	
	RelaxedApplicabilityManager() {}

	//!
	std::pair<bool, FactSetPtr> isApplicable(const Action& action, const State& seed, const DomainMap& domains) const;
	
protected:
	bool isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, FactSetPtr causes) const;
};

} // namespaces
