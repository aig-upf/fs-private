
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
public:
	typedef std::unique_ptr<const RelaxedEffectManager> ucptr;
	
	RelaxedEffectManager() {}

	//!
	void computeChangeset(const Action& action, const DomainMap& domains, Changeset& changeset) const;
	
protected:
	void computeUnaryChangeset(const ScopedEffect::cptr effect, VariableIdx relevant, ObjectIdx value, Changeset& changeset) const;
// 	void computeProcedurePointChangeset(const ScopedEffect::cptr effect, const VariableIdxVector& relevant, const ObjectIdxVector& values, Changeset& changeset) const;
};

} // namespaces
