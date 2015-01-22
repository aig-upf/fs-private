
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

/**
 * A RelaxedEffectManager is in charge of managing the effects of actions on a delete-free relaxation of the problem.
 */
class RelaxedEffectManager
{
protected:
	//! The state constraints
	const ScopedConstraint::vcptr& _constraints;

public:
	typedef std::unique_ptr<const RelaxedEffectManager> ucptr;
	
	RelaxedEffectManager(const ScopedConstraint::vcptr& constraints)
		: _constraints(constraints) {}

	//!
	void computeChangeset(const Action& action, const DomainMap& domains, Changeset& changeset) const;
	
protected:
	void computeProcedurePointChangeset(const ScopedEffect::cptr effect, const VariableIdxVector& relevant, const ObjectIdxVector& values, Changeset& changeset) const;
};

} // namespaces
