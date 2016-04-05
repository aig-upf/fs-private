
#pragma once

#include <fs0_types.hxx>
#include <utils/binding.hxx>


namespace fs0 { namespace language { namespace fstrips { class Formula; class ActionEffect; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ActionSchema;
class ApplicableActionSet;
class ProblemInfo;
class GroundAction;
class Binding;

//! A base interface for the different types of actions that the planners deals with:
//! lifted and grounded actions
class BaseAction {
protected:
	//! The id that identifies the concrete action within the set of actions of the same type
	//! (i.e. a grounded and a lifted action might have the same ID and yet be different entities)
	unsigned _id;
	
	//! The action preconditions  and effects
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:
	typedef const BaseAction* cptr;
	
	BaseAction(unsigned id, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	virtual ~BaseAction();
	
	//! Returns the name of the action, e.g. 'move'
	virtual const std::string& getName() const = 0;
	
	unsigned getId() const { return _id; }
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	virtual std::string fullname() const = 0;
	
	//! Returns the signature of the action
	virtual const Signature& getSignature() const = 0;
	
	//! Return true iff the i-th action parameter is bound
	virtual bool isBound(unsigned i) const = 0;
	
	const fs::Formula* getPrecondition() const { return _precondition; }
	
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const BaseAction&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;
};


class ActionSchema : public BaseAction {
protected:
	const std::string _name;
	const Signature _signature;
	const std::vector<std::string> _parameters;

public:
	typedef const ActionSchema* cptr;
	ActionSchema(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameters,
			     const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	
	~ActionSchema() {}
	
	const std::string& getName() const { return _name; }
	
	const Signature& getSignature() const { return _signature; }
	
	const std::vector<std::string>& getParameters() const { return _parameters; }
	
	std::string fullname() const;
	
	bool isBound(unsigned i) const { return false; } // On an ActionSchema, all parameters are unbound.

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	//! Process the action schema with a given parameter binding and return the corresponding GroundAction
	//! A nullptr is returned if the action is detected to be statically non-applicable
	GroundAction* bind(unsigned id, const Binding& binding, const ProblemInfo& info) const;
	GroundAction* bind(const Binding& binding, const ProblemInfo& info) const;
	
	//! This processes in-place the schema to consolidate state variables, etc.
	ActionSchema* process(const ProblemInfo& info) const;
};

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
	
	GroundAction(unsigned id, const ActionSchema* schema, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~GroundAction() {}
	
	//! Returns the name of the action, e.g. 'move'
	const std::string& getName() const;
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	std::string fullname() const;
	
	bool isBound(unsigned i) const;
	
	//! Returns the signature of the action
	const Signature& getSignature() const;
	
	//! Returns the concrete binding that created this action from its action schema
	const Binding& getBinding() const { return _binding; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
};



} // namespaces
