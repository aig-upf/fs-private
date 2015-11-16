
#pragma once

#include <actions/base_action.hxx>

namespace fs0 {

class ProblemInfo; class GroundAction; class Binding;

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

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	//! Process the action schema with a given parameter binding and return the corresponding GroundAction
	//! A nullptr is returned if the action is detected to be statically non-applicable
	GroundAction* bind(const Binding& binding, const ProblemInfo& info) const;
	
	//! This processes in-place the schema to consolidate state variables, etc.
	ActionSchema* process(const ProblemInfo& info) const;
};


} // namespaces
