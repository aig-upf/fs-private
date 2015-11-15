
#include <limits>
#include <sstream>

#include <actions/ground_action.hxx>
#include <actions/action_schema.hxx>
#include <utils/printers/actions.hxx>

namespace fs0 {

const ActionIdx GroundAction::invalid_action_id = std::numeric_limits<unsigned int>::max();

GroundAction::GroundAction(ActionSchema::cptr schema, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects)
	: BaseAction(invalid_action_id, precondition, effects), _schema(schema), _binding(binding)
{}


const std::string& GroundAction::getName() const { return _schema->getName(); }

const Signature& GroundAction::getSignature() const { return _schema->getSignature(); }

std::string GroundAction::fullname() const {
     std::ostringstream stream;
     stream << print::action_name(*this);
     return stream.str();
}

std::ostream& GroundAction::print(std::ostream& os) const {
	os << print::action_name(*this) << std::endl;
	return BaseAction::print(os);
}


} // namespaces
