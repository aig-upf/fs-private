
#pragma once

#include <actions.hxx>
#include <fs0_types.hxx>
#include <state.hxx>
#include "constraints/constraint_manager.hxx"

namespace fs0 {

class Problem; class RPGData;


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
class ActionManagerFactory
{
public:
	//! A factory method to create the appropriate manager.
	static void instantiateActionManager(const Action::vcptr& actions);
	
	//! Returns true iff at least one of the preconditions of the action has arity higher than one.
	static bool checkActionHasNaryPreconditions(const Action::cptr action);
	
	//! Returns true iff at least one of the effects of the action has arity higher than one.
	static bool checkActionHasNaryEffects(const Action::cptr action);
};



/**
 * Interface for an action manager.
 */
class BaseActionManager {
public:
	
	BaseActionManager(bool hasNaryEffects) : _hasNaryEffects(hasNaryEffects) {};
	virtual ~BaseActionManager() {};
	
	//! 
	void processAction(unsigned actionIdx, const Action& action, const State& seed, const RelaxedState& layer, RPGData& changeset) const;
	
	//!
	virtual bool checkPreconditionApplicability(const Action& action, const State& seed, const DomainMap& domains, Atom::vctr& causes) const = 0;
	
	//!
	void processEffects(unsigned actionIdx, const Action& action, Atom::vctrp actionSupport, const State& seed, const DomainMap& actionProjection, RPGData& rpgData) const;
	
protected:
	bool _hasNaryEffects;
};


/**
 * A constraint manager capable only of dealing with actions whose constraints are all unary,
 * in which case the handling is much simpler and more efficient.
 */
class UnaryActionManager : public BaseActionManager
{
public:
	//! Constructs a manager handling the given set of constraints
	UnaryActionManager(const Action& action, bool naryEffects)
		: BaseActionManager(naryEffects) {}
	~UnaryActionManager() {}
	
	bool checkPreconditionApplicability(const Action& action, const State& seed, const DomainMap& domains, Atom::vctr& causes) const;
	
protected:
	static bool isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, Atom::vctr& causes);
};


class GenericActionManager : public BaseActionManager
{
protected:
	//! 
	const ConstraintManager manager;
	
public:
	//! Constructs a manager handling the given set of constraints
	GenericActionManager(const Action& action, bool naryEffects)
		:  BaseActionManager(naryEffects) ,manager(action.getConstraints()) {}
	~GenericActionManager() {}
	
	bool checkPreconditionApplicability(const Action& action, const State& seed, const DomainMap& domains, Atom::vctr& causes) const;
};

} // namespaces
