
#include "action_csp.hxx"
#include <problem.hxx>
#include <gecode/driver.hh>
#include <set>
#include <algorithm>

using namespace Gecode;

namespace fs0 { namespace gecode {

ActionCSP::ActionCSP( bool share, ActionCSP& other ) : 
	Gecode::Space( share, other )
{
	_X.update( *this, share, other._X );
	_Y.update( *this, share, other._Y );
}

//! Shallow copy operator, see notes on search in Gecode to
//! get an idea of what is being "actually" copied
Gecode::Space* ActionCSP::copy( bool share ) {
	return new ActionCSP( share, *this );
}

//! Standard copy constructor
ActionCSP::ActionCSP(ActionCSP& other) : 
	Gecode::Space(other), 
	_X(other._X),
	_Y(other._Y)
{}
	
bool ActionCSP::checkConsistency() {
	Gecode::SpaceStatus st = status();
	return st != Gecode::SpaceStatus::SS_FAILED;
}

/*
void ActionCSP::retrieveAtomsForAffectedVariable( VariableIdx varName, Atom::vctr& atoms ) const {
	auto x = resolveY( varName );
	for (Gecode::IntVarValues i(x); i(); ++i)
	atoms.push_back( Atom( varName, i.val() ));
}

void ActionCSP::retrieveRangesForAffectedVariable(VariableIdx varName, Atom::vrange& ranges) const {
	auto x = resolveY( varName );
	for (Gecode::IntVarRanges i(x); i(); ++i)
	ranges.push_back( std::make_tuple( Atom( varName, i.min()), Atom( varName, i.max()) ) );
}
*/
} } // namespaces
