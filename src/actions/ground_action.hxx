
#pragma once

#include <fs0_types.hxx>
#include <actions/action_schema.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {

class ApplicableActionSet;
class BaseActionManager;

class GroundAction {
protected:
	//! The schema from which the action was grounded
	ActionSchema::cptr _schema;
	
	//! The indexes of the action binding, if any.
	const ObjectIdxVector _binding;
	
	//! The action preconditions  and effects
	const std::vector<AtomicFormula::cptr> _conditions;
	const std::vector<ActionEffect::cptr> _effects;

public:
	typedef const GroundAction* cptr;
	
	//! Trait required by aptk::DetStateModel
	typedef ActionIdx IdType;
	typedef ApplicableActionSet ApplicableSet;

	static const ActionIdx invalid_action_id;
	
	GroundAction(ActionSchema::cptr schema, const ObjectIdxVector& binding, const std::vector<AtomicFormula::cptr>& conditions, const std::vector<ActionEffect::cptr>& effects);
	~GroundAction();
	
	//! Returns the name of the action, e.g. 'move'
	const std::string& getName() const { return _schema->getName(); }
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	std::string getFullName() const;
	
	//! Returns the signature of the action
	const Signature& getSignature() const { return _schema->getSignature(); }
	
	//! Returns the concrete binding that created this action from its action schema
	const ObjectIdxVector& getBinding() const { return _binding; }
	
	inline const std::vector<AtomicFormula::cptr>& getConditions() const { return _conditions; }
	
	inline const std::vector<ActionEffect::cptr>& getEffects() const { return _effects; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GroundAction&  entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};


} // namespaces
