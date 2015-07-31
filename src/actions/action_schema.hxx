
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/schemata.hxx>


using namespace fs0::language::fstrips;

namespace fs0 {

class ProblemInfo; class GroundAction;

class ActionSchema {
protected:
	const std::string _name;
	const std::string _classname;
	const std::vector<TypeIdx> _signature;
	const std::vector<std::string> _parameters;
	const std::vector<AtomicFormulaSchema::cptr> _conditions;
	const std::vector<ActionEffectSchema::cptr> _effects;
	
	

public:
	typedef const ActionSchema* cptr;
	ActionSchema(const std::string& name, const std::string& classname,
				 const std::vector<TypeIdx>& signature, const std::vector<std::string>& parameters,
			     const std::vector<AtomicFormulaSchema::cptr>& conditions, const std::vector<ActionEffectSchema::cptr>& effects);
	~ActionSchema();
	
	inline const std::string& getName() const { return _name; }
	inline const std::string& getClassname() const { return _classname; }
	inline const std::vector<TypeIdx>& getSignature() const { return _signature; }
	inline const std::vector<std::string>& getParameters() const { return _parameters; }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	//! Process the action schema with a given parameter binding and return the corresponding GroundAction
	//! A nullptr is returned if the action is detected to be statically non-applicable
	GroundAction* process(const ObjectIdxVector& binding, const ProblemInfo& info) const;
};


} // namespaces
