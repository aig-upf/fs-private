
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
 * (1) In the simplest case, when all of the action preconditions and effects are at most unary
 *     (effects can be 0-ary, but note that 0-ary preconditions are static and should be detected at compile-time),
 *     we use an exact method implemented by the UnaryActionManager.
 *
 * (2) In the general case, i.e. when the above restrictions on the arity of the procedures do not hold,
 *     we use a ComplexActionManager that involves a Gecode model.
 */
class ActionManagerFactory
{
public:
	//! A factory method to create the appropriate manager.
	static void instantiateActionManager(const Problem& problem, const Action::vcptr& actions);

protected:
	//! Returns true iff at least one of the preconditions of the action has arity higher than one.
	static bool actionHasHigherArityPreconditions(const Action::cptr action);

	//! Returns true iff at least one of the effects of the action has arity higher than one.
	static bool actionHasHigherArityEffects(const Action::cptr action);
};



/**
 * Base interface for an action manager.
 */
class BaseActionManager {
public:
	//!
	BaseActionManager() {};
	virtual ~BaseActionManager() {};

	//! Process the given action in the given layer of the RPG, adding newly-achievable atoms and their supoprts to the rpg data structure.
	virtual void processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg) = 0;
};


/**
 * A constraint manager capable only of dealing with actions whose constraints are all unary,
 * in which case the handling is much simpler and more efficient.
 */
class UnaryActionManager : public BaseActionManager
{
public:
	//! Constructs a manager handling the given set of constraints
	UnaryActionManager(const Action& action)
		:  BaseActionManager(), manager(action.getConstraints()) {}
	~UnaryActionManager() {}

	virtual void processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg);

	//!
	virtual bool checkPreconditionApplicability(const DomainMap& domains) const;

	//!
	void processEffects(unsigned actionIdx, const Action& action, const DomainMap& actionProjection, RPGData& rpg) const;

protected:
	//!
	const ConstraintManager manager;

	//!
	void completeAtomSupport(const VariableIdxVector& actionScope, const DomainMap& actionProjection, const VariableIdxVector& effectScope, Atom::vctrp support) const;
};


// This is probably completely subsumed by the ComplexActionManager and should be erased.
//
// class GenericActionManager : public BaseActionManager
// {
// public:
// 	//! Constructs a manager handling the given set of constraints
// 	GenericActionManager(const Action& action, bool naryEffects)
// 		:  BaseActionManager(action, naryEffects) {}
// 	~GenericActionManager() {}
//
// 	//! For actions with higher-arity applicability constraints, we need to thoroughly check for applicability of every element of the Cartesian product.
// 	virtual bool isCartesianProductElementApplicable(const VariableIdxVector& actionScope, const VariableIdxVector& effectScope, const DomainMap& actionProjection, const ObjectIdxVector& element, Atom::vctrp support) const;
// };

} // namespaces
