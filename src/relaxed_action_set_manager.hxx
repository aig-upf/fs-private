
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <fact.hxx>
#include "constraints/scoped_constraint.hxx"
#include <utils/utils.hxx>

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
	std::pair<bool, FactSetPtr> isApplicable(const ApplicableEntity& entity, const DomainMap& domains) const;
	
	//!
	void computeChangeset(const CoreAction& action, const DomainMap& domains, Changeset& changeset) const;
	
protected:
	bool isProcedureApplicable(const ApplicableEntity& entity, const DomainMap& domains, unsigned procedureIdx, FactSetPtr causes) const;
	
	void computeProcedureChangeset(unsigned procedureIdx, const CoreAction& action, const DomainMap& domains, Changeset& changeset) const;
	void computeProcedurePointChangeset(unsigned procedureIdx, const CoreAction& action,
										const VariableIdxVector& relevant, const ProcedurePoint& point, Changeset& changeset) const;
};

} // namespaces
