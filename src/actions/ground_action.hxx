
#pragma once

#include <fs0_types.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; class ActionEffect; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ActionSchema;
class ApplicableActionSet;

class GroundAction {
protected:
	//! The schema from which the action was grounded
	const ActionSchema* _schema;
	
	//! The indexes of the action binding, if any.
	const Binding _binding;
	
	//! The action preconditions  and effects
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:
	typedef const GroundAction* cptr;
	
	//! Trait required by aptk::DetStateModel
	typedef ActionIdx IdType;
	typedef ApplicableActionSet ApplicableSet;

	static const ActionIdx invalid_action_id;
	
	GroundAction(const ActionSchema* schema, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~GroundAction();
	
	//! Returns the name of the action, e.g. 'move'
	const std::string& getName() const;
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	std::string getFullName() const;
	
	//! Returns the signature of the action
	const Signature& getSignature() const;
	
	//! Returns the concrete binding that created this action from its action schema
	const Binding& getBinding() const { return _binding; }
	
	const fs::Formula* getPrecondition() const { return _precondition; }
	
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GroundAction&  entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};


} // namespaces
