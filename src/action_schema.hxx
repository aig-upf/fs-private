
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>

using namespace fs0::language::fstrips;

namespace fs0 {

class ActionSchema {
protected:
	const std::string _name;
	const std::string _classname;
	const std::vector<AtomicFormulaSchema::cptr> _conditions;
	const std::vector<ActionEffectSchema::cptr> _effects;
	
	

public:
	typedef const ActionSchema* cptr;
	ActionSchema(const std::string& name, const std::string& classname, const std::vector<AtomicFormulaSchema::cptr>& conditions, const std::vector<ActionEffectSchema::cptr>& effects);
	~ActionSchema();


};


} // namespaces
