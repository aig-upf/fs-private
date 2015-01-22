
#pragma once

#include <iosfwd>
#include <limits>
#include <memory>
#include <vector>

#include <fs0_types.hxx>
#include <state.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>
#include "constraints/scoped_effect.hxx"

namespace fs0 {

class Changeset;

class Action
{
protected:
	
	//! The indexes of the action binding, if any.
	const ObjectIdxVector _binding;
	
	//! One VariableIdxVector per each applicability procedure, containing the indexes of those state variables 
	//! relevant to that procedure.
	const ScopedConstraint::vcptr _constraints;
	
	const ScopedEffect::vcptr _effects;
	
	//! The indexes of _all_ the state variables relevant to at least one of the effect or applicability procedures of the action.
	const VariableIdxVector _allRelevantVars;
	
public:
	static const ActionIdx INVALID;
	
	typedef std::shared_ptr<const Action> cptr;
	typedef std::vector<Action::cptr> vcptr;
	
	Action(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects);
	
	//! Keep it virtual!
	virtual ~Action();
	
	inline unsigned getNumConstraints() const { return _constraints.size(); }
	
	inline unsigned getNumEffects() const { return _effects.size(); }
	
	const VariableIdxVector& getAllRelevantVariables() const { return _allRelevantVars; }
	
	//! Returns the name of the action. To be implemented in each concrete action.
	virtual const std::string& getName() const = 0;
	
	//! Returns the signature of the action. To be implemented in each concrete action.
	//! An action signature is a vector v of types, where v[i] denotes the type of the action's i-th parameter.
	virtual const ActionSignature& getSignature() const = 0;
	
	inline const ScopedConstraint::vcptr getConstraints() const { return _constraints; }
	
	inline const ScopedEffect::vcptr getEffects() const { return _effects; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Action&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;

protected:
	//!
	VariableIdxVector extractRelevantVariables();
};


} // namespaces
