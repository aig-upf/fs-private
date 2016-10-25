
#pragma once

#include <fs_types.hxx>
#include <utils/binding.hxx>
#include <actions/ground_action_iterator.hxx>


namespace fs0 { namespace language { namespace fstrips { class Term; class Formula; class ActionEffect; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ProblemInfo;

//! All the data that fully characterizes a lifted action
class ActionData {
protected:
	//! The ID of the original action schema (not to be confused with the ID of resulting fully-grounded actions)
	unsigned _id;
	
	const std::string _name;
	const Signature _signature;
	const std::vector<std::string> _parameter_names;
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:
	ActionData(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameter_names,
			   const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	
	~ActionData();
	ActionData(const ActionData&);
	
	unsigned getId() const { return _id; }
	const std::string& getName() const { return _name; }
	const Signature& getSignature() const { return _signature; }
	const std::vector<std::string>& getParameterNames() const { return _parameter_names; }
	const fs::Formula* getPrecondition() const { return _precondition; }
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }
	
	//! Returns true if some parameter of the action has a type with no associated object
	bool has_empty_parameter() const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionData& entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};


//! A base interface for the different types of actions that the planners deals with:
//! lifted and grounded actions
class ActionBase {
protected:
	//! The data of the action schema that has originated this action
	const ActionData& _data;

	//! The action binding (which will be an empty binding for fully-lifted actions, 
	//! and a full binding for fully-grounded actions, all steps in between being possible).
	const Binding _binding;
	
	//! The action preconditions and effects, perhaps partially grounded
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:
	
	ActionBase(const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	virtual ~ActionBase();
	
	ActionBase(const ActionBase& o);
	
	const ActionData& getActionData() const { return _data; };
	
	//! Some method redirections
	unsigned getOriginId() const { return _data.getId(); }
	const std::string& getName() const { return _data.getName(); }
	const Signature& getSignature() const { return _data.getSignature(); }
	const std::vector<std::string>& getParameterNames() const { return _data.getParameterNames(); }
	unsigned numParameters() const { return getSignature().size(); }
	
	const fs::Formula* getPrecondition() const { return _precondition; }
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }
	
	//!
	const Binding& getBinding() const { return _binding; }
	
	//! Return true iff the i-th action parameter is bound
	bool isBound(unsigned i) const { return _binding.binds(i); }
	
	//! Returns true if some parameter of the action has a type with no associated object
	bool has_empty_parameter() const { return _data.has_empty_parameter(); }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionBase&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;
};

//! An action that can be partially grounded (or fully lifted)
class PartiallyGroundedAction : public ActionBase {
public:
	PartiallyGroundedAction(const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~PartiallyGroundedAction() = default;
	PartiallyGroundedAction(const PartiallyGroundedAction&) = default;
};


//! A fully-grounded action can get an integer ID for more performant lookups
class GroundAction : public ActionBase {
protected:
	//! The id that identifies the concrete action within the whole set of ground actions
	unsigned _id;

public:
	//! Trait required by aptk::DetStateModel
	using IdType = ActionIdx;
	using ApplicableSet = GroundActionIterator;

	static const ActionIdx invalid_action_id;
	
	GroundAction(unsigned id, const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~GroundAction() = default;
	GroundAction(const GroundAction&) = default;
	
	unsigned getId() const { return _id; }
};





} // namespaces
