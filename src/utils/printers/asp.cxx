
#include <utils/printers/asp.hxx>
#include <clingo/clingocontrol.hh>

namespace fs0 { namespace print {

std::ostream& asp_model::print(std::ostream& os) const {
	for (auto &atom : _model) {
		os << atom << " ";
	}
	return os;
}

} } // namespaces
