
#pragma once

#include <fs0_types.hxx>
#include <actions/action_schema.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {

class ApplicableActionSet;

class GroundAction {
protected:
	//! The schema from which the action was grounded
	ActionSchema::cptr _schema;
	
	//! The indexes of the action binding, if any.
	const Binding _binding;
	
	//! The action preconditions  and effects
	const Formula::cptr _precondition;
	const std::vector<ActionEffect::cptr> _effects;

public:
	typedef const GroundAction* cptr;
	
	//! Trait required by aptk::DetStateModel
	typedef ActionIdx IdType;
	typedef ApplicableActionSet ApplicableSet;

	static const ActionIdx invalid_action_id;
	
	GroundAction(ActionSchema::cptr schema, const Binding& binding, const Formula::cptr precondition, const std::vector<ActionEffect::cptr>& effects);
	~GroundAction();
	
	//! Returns the name of the action, e.g. 'move'
	const std::string& getName() const { return _schema->getName(); }
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	std::string getFullName() const;
	
	//! Returns the signature of the action
	const Signature& getSignature() const { return _schema->getSignature(); }
	
	//! Returns the concrete binding that created this action from its action schema
	const Binding& getBinding() const { return _binding; }
	
	const Formula::cptr getPrecondition() const { return _precondition; }
	
	const std::vector<ActionEffect::cptr>& getEffects() const { return _effects; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GroundAction&  entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};


} // namespaces
