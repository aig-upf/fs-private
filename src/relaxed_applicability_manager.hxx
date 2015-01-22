
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <heuristics/changeset.hxx>
#include <fs0_types.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

/**
 * An applicability manager operating on a delete-free relaxation of the problem in order to 
 * assess the applicability of actions (disregarding state constraints is part of the relaxation).
 * 
 * There are two types of applicability manager:
 * (1) The UnaryApplicabilityManager is a simplified version that works with actions whose applicability constraints are all unary.
 * (2) The GenericApplicabilityManager approximates the applicability of actions with applicability constraints of higher arity.
 *     To do so, it uses the same local consistency strategies than we use to check if a RPG layer is a goal layer.
 */
class RelaxedApplicabilityManager
{
public:
	//! A factory method to create the appropriate manager.
	static RelaxedApplicabilityManager* createApplicabilityManager(const Action::vcptr& actions);
	static bool checkCompleteApplicabilityManagerNeeded(const Action::vcptr& actions);
	
	RelaxedApplicabilityManager() {}
	virtual ~RelaxedApplicabilityManager() {}
	
	//!
	virtual std::pair<bool, FactSetPtr> isApplicable(const Action& action, const State& seed, const DomainMap& domains) const = 0;
};

class UnaryApplicabilityManager : public RelaxedApplicabilityManager
{
public:
	UnaryApplicabilityManager() : RelaxedApplicabilityManager() {}
	virtual ~UnaryApplicabilityManager() {}

	//!
	virtual std::pair<bool, FactSetPtr> isApplicable(const Action& action, const State& seed, const DomainMap& domains) const;
	
protected:
	virtual bool isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, FactSetPtr causes) const;
};

class GenericApplicabilityManager : public RelaxedApplicabilityManager
{
public:
	GenericApplicabilityManager() : RelaxedApplicabilityManager() {}
	virtual ~GenericApplicabilityManager() {}
	
	std::pair<bool, FactSetPtr> isApplicable(const Action& action, const State& seed, const DomainMap& domains) const;
};

} // namespaces
