
#include <constraints/gecode/simple_csp.hxx>


namespace fs0 { namespace gecode {

SimpleCSP::SimpleCSP() : _value_selector(nullptr) {}

SimpleCSP::~SimpleCSP() {}

//! Cloning constructor, required by Gecode
SimpleCSP::SimpleCSP( bool share, SimpleCSP& other ) :
	Gecode::Space(share, other),
	_value_selector(other._value_selector)
{
	_intvars.update( *this, share, other._intvars );
	_boolvars.update( *this, share, other._boolvars );
}

//! Shallow copy operator, see notes on search in Gecode to
//! get an idea of what is being "actually" copied
Gecode::Space* SimpleCSP::copy( bool share ) { return new SimpleCSP( share, *this ); }


bool SimpleCSP::checkConsistency() {
	Gecode::SpaceStatus st = status();
	return st != Gecode::SpaceStatus::SS_FAILED;
}

//! Prints a representation of a CSP. Mostly for debugging purposes
std::ostream& SimpleCSP::print(std::ostream& os) const {
	os << _intvars << std::endl;
	os << _boolvars; 
	return os;
}

void SimpleCSP::init_value_selector(std::shared_ptr<MinHMaxValueSelector> value_selector) {
	_value_selector = value_selector;
}

int SimpleCSP::select_value(Gecode::IntVar& x, int csp_var_idx) const {
	// If the value selector has not been initialized, we simply fall back to a min-value selection policy
	if (_value_selector == nullptr) return x.min();
	
	// Otherwise we forward the call to the appropriate value selector
	return _value_selector->select(x, csp_var_idx);
}

} } // namespaces
