
#include <action_schema.hxx>


namespace fs0 {

ActionSchema::ActionSchema(const std::vector<AtomicFormulaSchema::cptr>& conditions, const std::vector<ActionEffectSchema::cptr>& effects)
	: _conditions(conditions), _effects(effects)
{}


ActionSchema::~ActionSchema() {
	for (const auto ptr:_conditions) delete ptr;
	for (const auto ptr:_effects) delete ptr;
}


} // namespaces
