
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
	
	BaseActionManager(const Action& action, bool hasNaryEffects) : manager(action.getConstraints()), _hasNaryEffects(hasNaryEffects) {};
	virtual ~BaseActionManager() {};
	
	//! 
	void processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& changeset) const;
	
	//!
	virtual bool checkPreconditionApplicability(const DomainMap& domains) const;
	
	//!
	void processEffects(unsigned actionIdx, const Action& action, const DomainMap& actionProjection, RPGData& rpgData) const;
	
	//! By default, any element of the Cartesian product of 
	virtual bool isCartesianProductElementApplicable(const VariableIdxVector& actionScope, const VariableIdxVector& effectScope, const DomainMap& actionProjection, const ObjectIdxVector& element, Atom::vctrp support) const = 0;
	
protected:
	//! 
	const ConstraintManager manager;
	
	//!
	bool _hasNaryEffects;
	
	void completeAtomSupport(const VariableIdxVector& actionScope, const DomainMap& actionProjection, const VariableIdxVector& effectScope, Atom::vctrp support) const;
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
		:  BaseActionManager(action, naryEffects) {}
	~UnaryActionManager() {}
	
	//! Any element of the Cartesian product is applicable when the constraints are all unary.
	bool isCartesianProductElementApplicable(const VariableIdxVector& actionScope, const VariableIdxVector& effectScope, const DomainMap& actionProjection, const ObjectIdxVector& element, Atom::vctrp support) const {
		return true;
	}
};


class GenericActionManager : public BaseActionManager
{
public:
	//! Constructs a manager handling the given set of constraints
	GenericActionManager(const Action& action, bool naryEffects)
		:  BaseActionManager(action, naryEffects) {}
	~GenericActionManager() {}
	
	//! For actions with higher-arity applicability constraints, we need to thoroughly check for applicability of every element of the Cartesian product.
	virtual bool isCartesianProductElementApplicable(const VariableIdxVector& actionScope, const VariableIdxVector& effectScope, const DomainMap& actionProjection, const ObjectIdxVector& element, Atom::vctrp support) const;
};

} // namespaces
