
#include <fs/core/constraints/gecode/v2/gecode_space.hxx>


namespace fs0::gecode::v2 {


FSGecodeSpace::FSGecodeSpace() :
        intvars(),
        boolvars()
{}

FSGecodeSpace::FSGecodeSpace(std::size_t numintvars, std::size_t numboolvars) :
    intvars(*this, numintvars),
    boolvars(*this, numboolvars)
{}

//! Cloning constructor, required by Gecode
FSGecodeSpace::FSGecodeSpace(FSGecodeSpace& other) :
    Gecode::Space(other)
{
	intvars.update(*this, other.intvars);
	boolvars.update(*this, other.boolvars);
}

Gecode::Space* FSGecodeSpace::copy() { return new FSGecodeSpace(*this); }


bool FSGecodeSpace::propagate() {
	return status() != Gecode::SpaceStatus::SS_FAILED;
}

//! Prints a representation of a CSP. Mostly for debugging purposes
std::ostream& FSGecodeSpace::print(std::ostream& os) const {
	os << intvars << std::endl;
	os << boolvars;
	return os;
}


} // namespaces
