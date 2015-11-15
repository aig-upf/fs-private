
#include <actions/ground_action.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <languages/fstrips/language.hxx>
#include <actions/action_schema.hxx>
#include <sstream>


namespace fs0 {

const ActionIdx GroundAction::invalid_action_id = std::numeric_limits<unsigned int>::max();

GroundAction::GroundAction(ActionSchema::cptr schema, const Binding& binding, const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects)
	: _schema(schema), _binding(binding), _precondition(precondition), _effects(effects)
{}


GroundAction::~GroundAction() {
	delete _precondition;
	for (const auto pointer:_effects) delete pointer;
}

const std::string& GroundAction::getName() const { return _schema->getName(); }

const Signature& GroundAction::getSignature() const { return _schema->getSignature(); }

std::string GroundAction::fullname() const {
     std::ostringstream stream;
     stream << print::action_name(*this);
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
