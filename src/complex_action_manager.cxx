
#include <cassert>
#include <iosfwd>
#include <complex_action_manager.hxx>
#include <constraints/constraint_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <boost/container/flat_map.hpp>
#include <constraints/gecode/action_csp.hxx>
#include <constraints/gecode/expr_translator_repository.hxx>
#include <tuple>

using namespace Gecode;
using namespace fs0::gecode;

namespace fs0 {

ComplexActionManager::ComplexActionManager(const Problem& problem, const Action& action)
	:  BaseActionManager()
{
	// MRJ: Very important - having the managers to be the ones "collecting" the constraints
	// from preconditions and actions, means that we need to make sure that the CSP being
	// referred to by the action manager is already built and assigned. Having addDefaultConstraints()
	// to be called from within createCSPVariables() will cause to refer to the baseCSP
	// attribute *before* it is properly initialized...
	baseCSP = createCSPVariables(action, problem.getConstraints());
	addDefaultConstraints( action, problem.getConstraints() );
	// MRJ: in order to be able to clone a CSP, we need to ensure that
	// it is "stable" i.e. propagate all constraints until fixed point
	baseCSP->status();
}

ComplexActionManager::~ComplexActionManager() {
	delete baseCSP;
}


void ComplexActionManager::processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& changeset) {
	// MRJ: This is rather ugly, the problem is that clone returns a pointer to Space...
	// it may be a good idea to overwrite this method on ActionCSP to avoid this down
	// cast.
	ActionCSP* currentCSP = dynamic_cast<ActionCSP::ptr>(baseCSP->clone());

	//std::cout << *currentCSP << std::endl;

	// Setup domain constraints etc.
	DomainMap actionProjection = Projections::projectToActionVariables(layer, action);
	for ( auto entry : actionProjection ) {
		VariableIdx x = entry.first;
		DomainPtr dom = entry.second;
		if ( dom->size() == 1 ) {
			addEqualityConstraint( *currentCSP, x, *(dom->begin()) );
		} else {
			ObjectIdx lb = *(dom->begin());
			ObjectIdx ub = *(dom->rbegin());
			// MRJ: Check this is a safe assumption
			if ( dom->size() == (ub - lb) ) {
				addBoundsConstraint( *currentCSP, x, lb, ub );
			} else { // MRJ: worst case (performance wise) yet I think it can be optimised in a number of ways
			addMembershipConstraint( *currentCSP, x, dom );
			}
		}
	}

	// Check local consistency
// 	std::cout << currentCSP << std::endl;
	if (!currentCSP->checkConsistency()) return; // We're done

	for ( ScopedEffect::cptr effect : action.getEffects() ) {
		Atom::vctrp support = std::make_shared<Atom::vctr>();

		// TODO (guillem): Add to the support all atoms arising from the values assigned to the relevant state variables
		// by the current CSP solution (or at least some solution)
		// Alternatively (we'll analyze the performance of both options) randomly select one value from the set of locally consistent values
		// of each variable.

	//         rpg.add(effect->apply(values), actionIdx, support);
	}

}

const IntVar&  ComplexActionManager::resolveX( VariableIdx varName ) const {
	return resolveVariableName( varName, baseCSP->_X, inputVariables );
}

const IntVar&  ComplexActionManager::resolveY( VariableIdx varName ) const {
	return resolveVariableName( varName, baseCSP->_Y, outputVariables );
}

const IntVar& ComplexActionManager::resolveVariableName( VariableIdx varName, const IntVarArray& actualVars, const VariableMap& map ) const {
	auto it = map.find( varName );
	assert( it != map.end() );
	return actualVars[ it->second ];
}


/* MRJ: See comment in support_csp.hxx
void ActionCSP::addEqualityConstraint( VariableIdx v, int value ) {
	auto it = inputVariables.find( v );
	assert( it != inputVariables.end() );
	rel( *this, _X[ it->second ], IRT_EQ, value );
}
*/

void ComplexActionManager::addEqualityConstraint(ActionCSP& csp, VariableIdx v, bool value) {
	auto it = inputVariables.find( v );
	assert( it != inputVariables.end() );
	rel( csp, csp._X[ it->second ], IRT_EQ, (value ? 1 : 0) ); // v = value
}

void ComplexActionManager::addEqualityConstraint(ActionCSP& csp, VariableIdx v, ObjectIdx value) {
	auto it = inputVariables.find( v );
	assert( it != inputVariables.end() );
	rel( csp, csp._X[ it->second ], IRT_EQ, value ); // v = value
}


void ComplexActionManager::addBoundsConstraint(ActionCSP& csp, VariableIdx v, int lb, int ub) {
	auto it = inputVariables.find( v );
	assert( it != inputVariables.end() );
	dom( csp, csp._X[ it->second], lb, ub); // MRJ: lb <= v <= ub
}

void ComplexActionManager::addBoundsConstraintFromDomain(ActionCSP& csp, VariableIdx v) {
	auto it = inputVariables.find( v );
	assert( it != inputVariables.end() );
	const auto& info = Problem::getCurrentProblem()->getProblemInfo();
	TypeIdx type = info.getVariableType(v);
	if (!info.hasVariableBoundedDomain(type)) return; // Nothing to do in this case
	const auto& bounds = info.getVariableBounds(type);
	dom( csp, csp._X[ it->second], bounds.first, bounds.second); // MRJ: bounds.first <= v <= bounds.second
}

void ComplexActionManager::addMembershipConstraint(ActionCSP& csp, VariableIdx v, DomainPtr varDomain) {
	auto it = inputVariables.find( v );
	assert( it != inputVariables.end() );
	// MRJ: v \in dom
	TupleSet valueSet;
	for ( auto v : *varDomain ) {
		valueSet.add( IntArgs( 1, v ));
	}
	valueSet.finalize();
	extensional( csp, IntVarArgs() << csp._X[ it->second ], valueSet ); // MRJ: v \in valueSet
}


ActionCSP::ptr ComplexActionManager::createCSPVariables( const Action& a, const ScopedConstraint::vcptr& stateConstraints ) {
	// Determine input and output variables for this action: we first amalgamate variables into a set
	// to avoid repetitions, then generate corresponding CSP variables, then create the CSP model with them
	// and finally add the model constraints.
	VariableIdxSet inputVars, outputVars;

	// Add the variables mentioned by state constraints
// 	for ( ScopedConstraint::cptr global : stateConstraints ) {
// 		inputVars.insert( global->getScope().begin(), global->getScope().end() );
// 	}

	// Add the variables mentioned in the preconditions
	for ( ScopedConstraint::cptr prec : a.getConstraints() ) {
		inputVars.insert( prec->getScope().begin(), prec->getScope().end() );
	}

	// Add the variables appearing in the scope of the effects
	for ( ScopedEffect::cptr eff : a.getEffects() ) {
		inputVars.insert( eff->getScope().begin(), eff->getScope().end() );
		outputVars.insert( eff->getAffected() );
	}

	ActionCSP::ptr csp = new ActionCSP;

	IntVarArgs relevant;
	for (VariableIdx var:inputVars) {
		unsigned id = processVariable( *csp, var, relevant );
		inputVariables.insert( std::make_pair(var, id) );
	}

	IntVarArgs affected;
	for (VariableIdx var:outputVars) {
		unsigned id = processVariable( *csp, var, affected );
		outputVariables.insert( std::make_pair(var, id) );
	}

	IntVarArray tmpX( *csp, relevant );
	csp->_X.update( *csp, false, tmpX );// = IntVarArray( *csp, relevant );
	IntVarArray tmpY(*csp, affected);
	csp->_Y.update( *csp, false, tmpY ); //= IntVarArray( *csp, affected );


	std::cout << "Created ActionCSP with input variables: " << csp->_X << " and output variables: " << csp->_Y << std::endl;

	return csp;
}

void
ComplexActionManager::addDefaultConstraints( const Action& a, const ScopedConstraint::vcptr& stateConstraints) {
	// Create constraints, once variables have been properly defined
	// MRJ: These constraints should always be translatable if they're in the action description
// 	for ( ScopedConstraint::cptr global : stateConstraints ) {
// 		auto transObj = TranslatorRepository::instance().getConstraintTranslator( typeid(*global) );
// 		assert( transObj != nullptr );
// 		transObj->addConstraint( global, *csp );
// 	}

	for ( ScopedConstraint::cptr prec : a.getConstraints() ) {
		auto transObj = gecode::TranslatorRepository::instance().getConstraintTranslator( typeid(*prec) );
		assert( transObj != nullptr );
		transObj->addConstraint( prec, *this );
	}

	for ( ScopedEffect::cptr eff : a.getEffects() ) {
		auto transObj = gecode::TranslatorRepository::instance().getEffectTranslator( typeid(*eff) );
		assert( transObj != nullptr );
		transObj->addConstraint( eff, *this );
	}

}

unsigned ComplexActionManager::processVariable( ActionCSP& csp, VariableIdx var, IntVarArgs& varArray ) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	ProblemInfo::ObjectType varType = info.getVariableGenericType( var );
	if ( varType == ProblemInfo::ObjectType::INT ) {
		auto bounds = info.getVariableBounds(var);
		IntVar x( csp, bounds.first, bounds.second );
		std::cout << "Created CSP variable for state variable " << var << ": " << x << std::endl;
		varArray << x;
	}
	else if ( varType == ProblemInfo::ObjectType::BOOL ) {
		varArray << IntVar( csp, 0, 1 );
	}
	else if ( varType == ProblemInfo::ObjectType::OBJECT ) {
		const ObjectIdxVector& values = info.getVariableObjects( var );
		std::vector<int> _values( values.size() );
		for ( unsigned j = 0; j < values.size(); j++ ) {
			_values[j] = values[j];
		}
		varArray << IntVar( csp, IntSet( _values.data(), values.size() ));
	}
	return varArray.size() - 1;
}


} // namespaces
