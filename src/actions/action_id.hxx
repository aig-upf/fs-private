
#pragma once

#include <fs0_types.hxx>
#include <actions/action_schema.hxx>

namespace fs0 {

//! An action is fully identified by the ID of the action schema and the values of its parameters,
//! i.e. its binding
class ActionID {
protected:
	//! The schema from which the action was grounded
	unsigned _schema_id;
	
	//! The indexes of the action binding.
	const std::vector<ObjectIdx> _binding;
	
public:
	static const ActionID invalid;
	
	ActionID(ActionSchema::cptr schema, const std::vector<ObjectIdx>& binding);
	ActionID(ActionSchema::cptr schema, const Binding& binding);
	
	//! Returns the concrete binding that created this action from its action schema
	const std::vector<ObjectIdx>& getBinding() const { return _binding; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionID&  entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};


} // namespaces
