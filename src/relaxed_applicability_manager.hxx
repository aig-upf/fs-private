
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

/**
 * An applicability manager operating on a delete-free relaxation of the problem in order to 
 * assess the applicability of actions (disregarding state constraints is part of the relaxation).
 * 
 * There are two possible ways of checking the applicability of an action:
 * (1) In the simplest case, when all of the action applicability procedures are unary, we use an exact method, `unaryApplicable`
 * (2) In the general case, when the above restriction on the arity of the procedures does not hold, 
 *     we approximates the applicability of the action using the same local consistency strategies 
 *     than we use to check if a RPG layer is a goal layer.
 */
class RelaxedApplicabilityManager
{
public:
	//! A factory method to create the appropriate manager.
	static RelaxedApplicabilityManager* createApplicabilityManager(const Action::vcptr& actions);
	static bool checkActionNeedsCompleteApplicabilityManager(const Action::cptr action);
	
	RelaxedApplicabilityManager() {}
	~RelaxedApplicabilityManager() {}
	
	//!
	bool isApplicable(const Action& action, const State& seed, const DomainMap& domains, Fact::vctr& causes) const;

protected:
	bool unaryApplicable(const Action& action, const State& seed, const DomainMap& domains, Fact::vctr& causes) const;
	bool genericApplicable(ConstraintManager* manager, const State& seed, const DomainMap& domains, Fact::vctr& causes) const;
	
	bool isUnaryProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, Fact::vctr& causes) const;
};



} // namespaces
