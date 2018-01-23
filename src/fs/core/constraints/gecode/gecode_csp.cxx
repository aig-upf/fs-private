
#include <fs/core/constraints/gecode/gecode_csp.hxx>
#include <fs/core/constraints/gecode/utils/value_selection.hxx>


namespace fs0 { namespace gecode {

GecodeCSP::GecodeCSP() : _value_selector(nullptr) {}

GecodeCSP::~GecodeCSP() = default;

//! Cloning constructor, required by Gecode
GecodeCSP::GecodeCSP( bool share, GecodeCSP& other ) :
	Gecode::Space(share, other),
	_value_selector(other._value_selector)
{
	_intvars.update( *this, share, other._intvars );
	_boolvars.update( *this, share, other._boolvars );
}

//! Shallow copy operator, see notes on search in Gecode to
//! get an idea of what is being "actually" copied
Gecode::Space* GecodeCSP::copy(bool share) { return new GecodeCSP(share, *this); }


bool GecodeCSP::propagate() {
	return status() != Gecode::SpaceStatus::SS_FAILED;
}

//! Prints a representation of a CSP. Mostly for debugging purposes
std::ostream& GecodeCSP::print(std::ostream& os) const {
	os << _intvars << std::endl;
	os << _boolvars; 
	return os;
}

void GecodeCSP::init_value_selector(std::shared_ptr<MinHMaxValueSelector> value_selector) {
	_value_selector = std::move(value_selector);
}

int GecodeCSP::select_value(Gecode::IntVar& x, int csp_var_idx) const {
	// If the value selector has not been initialized, we simply fall back to a min-value selection policy
	if (_value_selector == nullptr) return x.min();
	
	// Otherwise we forward the call to the appropriate value selector
	return _value_selector->select(x, static_cast<unsigned int>(csp_var_idx));
}

} } // namespaces
