
#pragma once

#include <iosfwd>
#include <limits>
#include <memory>
#include <vector>

#include <fs0_types.hxx>
#include <state.hxx>
#include <atoms.hxx>
#include <constraints/scoped_constraint.hxx>
#include "constraints/scoped_effect.hxx"

namespace fs0 {

class BaseActionManager;
class SimpleApplicableActionSet;

class Action
{
protected:
	
	//! The indexes of the action binding, if any.
	const ObjectIdxVector _binding;
	
	//! One VariableIdxVector per each applicability procedure, containing the indexes of those state variables 
	//! relevant to that procedure.
	ScopedConstraint::vcptr _constraints;
	
	const ScopedEffect::vcptr _effects;
	
	//! The indexes of _only_ those variables relevant to the applicability procedures of the action.
	const VariableIdxVector _scope;
	
	//! The indexes of _all_ the state variables relevant to at least one of the effect or applicability procedures of the action.
	const VariableIdxVector _allRelevantVars;
	
	//! Optionally, each action might have an associated constraint manager that handles its precondition constraints.
	//! Ownership of the pointer belongs to the action itself.
	BaseActionManager* _constraintManager;
	
public:

	//! Trait required by aptk::DetStateModel
	typedef	ActionIdx			IdType;
	typedef SimpleApplicableActionSet	ApplicableSet;

	static const ActionIdx INVALID;
	static const ActionIdx invalid_action_id;
	
	// typedef std::shared_ptr<const Action> cptr;
	typedef Action* ptr;
	typedef Action* cptr;
	typedef std::vector<Action::cptr> vcptr;
	
	Action(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects);
	
	//! Keep it virtual!
	virtual ~Action();
	
	inline unsigned getNumConstraints() const { return _constraints.size(); }
	
	inline unsigned getNumEffects() const { return _effects.size(); }
	
	const VariableIdxVector& getScope() const { return _scope; }
	const VariableIdxVector& getAllRelevantVariables() const { return _allRelevantVars; }
	
	//! Returns the name of the action. To be implemented in each concrete action.
	virtual const std::string& getName() const = 0;
	
	//! Returns the signature of the action. To be implemented in each concrete action.
	//! An action signature is a vector v of types, where v[i] denotes the type of the action's i-th parameter.
	virtual const ActionSignature& getSignature() const = 0;
	
	inline const ScopedConstraint::vcptr& getConstraints() const { return _constraints; }
	inline ScopedConstraint::vcptr& getConstraints() { return _constraints; }
	
	inline void addConstraint(ScopedConstraint::cptr constraint) { _constraints.push_back(constraint); }
	
	inline const ScopedEffect::vcptr getEffects() const { return _effects; }
	
	BaseActionManager* getConstraintManager() const { return _constraintManager; };
	void setConstraintManager(BaseActionManager* constraintManager) { _constraintManager = constraintManager; };
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Action&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;

protected:
	//! Helpers to initialize the action data structures
	VariableIdxVector extractScope();
	VariableIdxVector extractRelevantVariables();
};


} // namespaces
