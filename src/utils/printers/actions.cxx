
#include <problem.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/language.hxx>
#include <utils/printers/actions.hxx>
#include <utils/printers/binding.hxx>
#include <actions/actions.hxx>


namespace fs0 { namespace print {

std::ostream& action_data_name::print(std::ostream& os) const {
	os << _action.getName() << "(" << print::signature(_action.getParameterNames(), _action.getSignature()) << ")";
	return os;
}

std::ostream& action_data::print(std::ostream& os) const {
	os << action_data_name(_action) << std::endl;
	os << "\t" << "Precondition:" << *_action.getPrecondition() << std::endl;
	os << "\t" << "Effects:" << std::endl;
	for (auto elem:_action.getEffects()) os << "\t\t" << *elem << std::endl;
	return os;
}

std::ostream& action_header::print(std::ostream& os) const {
	os << _action.getName() << "(" << print::partial_binding(_action.getParameterNames(), _action.getBinding(), _action.getSignature()) << ")";
	return os;
}

std::ostream& full_action::print(std::ostream& os) const {
	os << action_header(_action) << std::endl;
	os << "\t" << "Precondition:" << *_action.getPrecondition() << std::endl;
	os << "\t" << "Effects:" << std::endl;
	for (auto elem:_action.getEffects()) os << "\t\t" << *elem << std::endl;
	return os;
}

} } // namespaces
