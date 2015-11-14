
#pragma once

#include <fs0_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; class ActionEffect; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ProblemInfo; class GroundAction; class Binding;

class ActionSchema {
protected:
	const std::string _name;
	const Signature _signature;
	const std::vector<std::string> _parameters;
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:
	typedef const ActionSchema* cptr;
	ActionSchema(const std::string& name,
				 const Signature& signature, const std::vector<std::string>& parameters,
			     const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~ActionSchema();
	
	inline const std::string& getName() const { return _name; }
	inline const Signature& getSignature() const { return _signature; }
	inline const std::vector<std::string>& getParameters() const { return _parameters; }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	//! Process the action schema with a given parameter binding and return the corresponding GroundAction
	//! A nullptr is returned if the action is detected to be statically non-applicable
	GroundAction* bind(const Binding& binding, const ProblemInfo& info) const;
};


} // namespaces
