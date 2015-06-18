
#include <atoms.hxx>
#include <problem.hxx>

namespace fs0 {

Atom::Atom(const VariableIdx variable, const ObjectIdx value, bool checkRelevance ) :
	_variable(variable), _value(value), _relevant(false)
{
	_relevant = ( checkRelevance ? Problem::getCurrentProblem()->isGoalRelevant( variable ) : true );

}

Atom::Atom( const Atom& other ) :
	_variable(other._variable), _value(other._value), _relevant(other._relevant )
{

}

Atom::Atom( Atom&& other ) :
	_variable(other._variable), _value(other._value), _relevant(other._relevant )
{

}

const Atom&
Atom::operator=( const Atom& other ) {
	if ( this == &other) return *this;
	_variable = other._variable;
	_value = other._value;
	_relevant = other._relevant;
	return *this;
}

Atom&
Atom::operator=( Atom&& other ) {
	if ( this == &other) return *this;
	_variable = other._variable;
	_value = other._value;
	_relevant = other._relevant;
	return *this;
}


std::ostream& Atom::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << "[" << problemInfo.getVariableName(_variable) << "=" << problemInfo.getObjectName(_variable, _value) << "]";
	return os;
}

} // namespaces
