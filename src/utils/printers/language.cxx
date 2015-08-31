
#include <utils/printers/language.hxx>

namespace fs0 { namespace print {

std::ostream& formula::print(std::ostream& os) const {
	os << _formula;
	return os;
}

} } // namespaces
