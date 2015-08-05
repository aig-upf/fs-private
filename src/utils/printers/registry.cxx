
#include <utils/printers/registry.hxx>
#include <problem.hxx>
#include <constraints/registry.hxx>
#include <boost/units/detail/utility.hpp>

namespace fs0 { namespace print {


std::ostream& logical_registry::print(std::ostream& os) const {
	
	os  << std::endl << "Component Repository";
	os  << std::endl << "--------------------" << std::endl;
	os << "Formula creators for symbol names: ";
	for (const auto& it:_registry._formula_creators) {
		os << it.first << ", ";
	}
	os << std::endl << std::endl;
	
	os << "Direct translators for logical elements: " << std::endl;
	for (const auto& it:_registry._direct_formula_translators) {
		os << "\t* " << boost::units::detail::demangle(it.first.name()) << std::endl;
	}
	os << std::endl;
	
	os << "Gecode translators for logical elements: " << std::endl;
	for (const auto& it:_registry._gecode_term_translators) {
		os << "\t* " << boost::units::detail::demangle(it.first.name()) << std::endl;
	}
	os << std::endl;
	
	for (const auto& it:_registry._gecode_formula_translators) {
		os << "\t* " << boost::units::detail::demangle(it.first.name()) << std::endl;
	}
	os << std::endl;
	
	return os;
}



} } // namespaces
