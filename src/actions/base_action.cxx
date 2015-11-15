
#include <actions/base_action.hxx>
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


BaseAction::BaseAction(unsigned id, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects)
	: _id(id), _precondition(precondition), _effects(effects)
{}


BaseAction::~BaseAction() {
	delete _precondition;
	for (const auto pointer:_effects) delete pointer;
}

std::ostream& BaseAction::print(std::ostream& os) const {
	os << "\t" << "Precondition:" << *_precondition <<  std::endl;
	os << "\t" << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t\t" << *elem << std::endl;
	return os;
}


} // namespaces
