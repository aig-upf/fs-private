
#include <actions/action_id.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>


namespace fs0 {

// const ActionID ActionID::invalid = ActionID();

ActionID::ActionID(ActionSchema::cptr schema, const std::vector<ObjectIdx>& binding) 
{}

// ActionID::ActionID(ActionSchema::cptr schema, const Binding& binding)
// 	: _schema(schema), _binding(binding), _precondition(precondition), _effects(effects)
// {}



std::ostream& ActionID::print(std::ostream& os) const {
	ActionSchema::cptr schema = Problem::getInstance().getActionSchemata()[_schema_id];
	os << schema->getName() << "(" << print::binding(getBinding(), schema->getSignature()) << ")";
	return os;
}


} // namespaces
