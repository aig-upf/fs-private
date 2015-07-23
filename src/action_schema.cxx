
#include <action_schema.hxx>


namespace fs0 {

ActionSchema::ActionSchema(const std::string& name, const std::string& classname, const std::vector<AtomicFormulaSchema::cptr>& conditions, const std::vector<ActionEffectSchema::cptr>& effects)
	: _name(name), _classname(classname), _conditions(conditions), _effects(effects)
{}


ActionSchema::~ActionSchema() {
	for (const auto ptr:_conditions) delete ptr;
	for (const auto ptr:_effects) delete ptr;
}


} // namespaces
