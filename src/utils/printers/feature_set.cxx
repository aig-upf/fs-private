
#include <utils/printers/feature_set.hxx>
#include <problem.hxx>
#include <constraints/registry.hxx>


namespace fs0 { namespace print {


std::ostream& feature_set::print(std::ostream& os) const {
	os << "{";
	for ( unsigned i = 0; i < _values.size(); i++ ) {
		os << _varnames[i] << " = " << _values[i] << ", ";
	}
	os << "}";
	
	return os;
}



} } // namespaces
