
#include <fs/core/utils/printers/language.hxx>
#include <fs/core/languages/fstrips/axioms.hxx>
#include <fs/core/utils/printers/binding.hxx>


namespace fs0 { namespace print {

std::ostream& axiom_header::print(std::ostream& os) const {
	return os << _axiom.getName() << "(" << print::signature(_axiom.getParameterNames(), _axiom.getSignature()) << ")";
}

} } // namespaces
