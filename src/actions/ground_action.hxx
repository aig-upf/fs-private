
#pragma once

#include <actions/base_action.hxx>
#include <utils/binding.hxx>

namespace fs0 {

class ActionSchema;
class ApplicableActionSet;

class GroundAction : public BaseAction {
protected:
	//! The schema from which the action was grounded
	const ActionSchema* _schema;
	
	//! The indexes of the action binding, if any.
	const Binding _binding;
	
public:
	typedef const GroundAction* cptr;
	
	//! Trait required by aptk::DetStateModel
	typedef ActionIdx IdType;
	typedef ApplicableActionSet ApplicableSet;

	static const ActionIdx invalid_action_id;
	
	GroundAction(const ActionSchema* schema, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~GroundAction() {}
	
	void set_id(unsigned id) { _id = id; }
	
	//! Returns the name of the action, e.g. 'move'
	const std::string& getName() const;
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	std::string fullname() const;
	
	//! Returns the signature of the action
	const Signature& getSignature() const;
	
	//! Returns the concrete binding that created this action from its action schema
	const Binding& getBinding() const { return _binding; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
};


} // namespaces
