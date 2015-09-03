
#include <atoms.hxx>
#include <problem.hxx>

namespace fs0 {

Atom::Atom(const VariableIdx variable, const ObjectIdx value ) :
	_variable(variable), _value(value)
{}

Atom::Atom( const Atom& other ) :
	_variable(other._variable), _value(other._value)
{}

Atom::Atom( Atom&& other ) :
	_variable(other._variable), _value(other._value)
{}

const Atom& Atom::operator=( const Atom& other ) {
	if ( this != &other) {
		_variable = other._variable;
		_value = other._value;
	}
	return *this;
}

Atom& Atom::operator=( Atom&& other ) {
	if ( this != &other) {
		_variable = other._variable;
		_value = other._value;
	}
	return *this;
}


std::ostream& Atom::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getInfo();
	os << "[" << problemInfo.getVariableName(_variable) << "=" << problemInfo.getObjectName(_variable, _value) << "]";
	return os;
}

} // namespaces
