
#include <utils/printers/actions.hxx>
#include <utils/printers/binding.hxx>
#include <actions/ground_action.hxx>
#include <actions/action_schema.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace print {

action_name::action_name(unsigned action_idx) : action_name(*Problem::getInstance().getGroundActions().at(action_idx)) {}

std::ostream& action_name::print(std::ostream& os) const {
	os << _action.getName() << "(" << print::binding(_action.getBinding(), _action.getSignature()) << ")";
	return os;
}

schema_name::schema_name(unsigned schema_idx) : schema_name(*Problem::getInstance().getActionSchemata().at(schema_idx)) {}

std::ostream& schema_name::print(std::ostream& os) const {
	os << _schema.getName() << "(" << print::signature(_schema.getParameters(), _schema.getSignature()) << ")" << std::endl;
	return os;
}

} } // namespaces
