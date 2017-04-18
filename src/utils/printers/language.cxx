
#include <utils/printers/language.hxx>
#include <languages/fstrips/axioms.hxx>
#include <utils/printers/binding.hxx>


namespace fs0 { namespace print {

std::ostream& axiom_header::print(std::ostream& os) const {
	return os << _axiom.getName() << "(" << print::signature(_axiom.getParameterNames(), _axiom.getSignature()) << ")";
}

} } // namespaces
