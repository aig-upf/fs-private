
#include <utils/printers/actions.hxx>
#include <utils/printers/binding.hxx>
#include <actions/ground_action.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace print {

std::ostream& action_name::print(std::ostream& os) const {
	os << _action.getName() << "(" << print::binding(_action.getBinding(), _action.getSignature()) << ")";
	return os;
}

} } // namespaces
