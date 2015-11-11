
#include <actions/ground_action.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>


namespace fs0 {

const ActionIdx GroundAction::invalid_action_id = std::numeric_limits<unsigned int>::max();

GroundAction::GroundAction(ActionSchema::cptr schema, const Binding& binding, const Formula::cptr precondition, const std::vector<ActionEffect::cptr>& effects)
	: _schema(schema), _binding(binding), _precondition(precondition), _effects(effects)
{}


GroundAction::~GroundAction() {
	delete _precondition;
	for (const auto pointer:_effects) delete pointer;
}

std::string GroundAction::getFullName() const {
     std::ostringstream stream;
     stream << getName() << "(" << print::binding(getBinding(), getSignature()) << ")";
     return stream.str();
}

std::ostream& GroundAction::print(std::ostream& os) const {
	os << print::action_name(*this) << std::endl;
	
	os << "\t" << "Precondition:" << *_precondition <<  std::endl;
	
	os << "\t" << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t\t" << *elem << std::endl;
		
	return os;
}


} // namespaces
