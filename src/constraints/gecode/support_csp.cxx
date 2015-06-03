#include <constraints/gecode/support_csp.hxx>
#include <gecode/driver.hh>
#include <set>
#include <algorithm>

// MRJ: I don't love doing this
using namespace Gecode;

namespace fs0 {

  static void
  processVariable( SupportCSP& csp, const ProblemInfo& info, VariableIdx var, IntVarArgs& varArray ) {
      ProblemInfo::ObjectType varType = info.getVariableGenericType( var );
      if ( varType == ProblemInfo::ObjectType::INT ) {
        auto bounds = info.getVariableBounds(var);
        varArray << IntVar( csp, bounds.first, bounds.second );
      }
      else if ( varType == ProblemInfo::ObjectType::BOOL ) {
        varArray << IntVar( csp, 0, 1 );
      }
      else if ( varType == ProblemInfo::ObjectType::OBJECT ) {
        const ObjectIdxVector& values = info.getVariableObjects( var );
        std::vector<int> _values( values.size() );
        for ( unsigned j = 0; j < values.size(); j++ )
          _values[j] = values[j];
        varArray << IntVar( csp, IntSet( _values.data(), values.size() ));
      }
  }

  SupportCSP::SupportCSP( const ProblemInfo& info, const VariableIdxVector& inputVars, const VariableIdxVector& outputVars) {
    IntVarArgs tmpX;
    for ( unsigned k = 0; k < inputVars.size(); k++ ) {
      processVariable( *this, info, inputVars[k], tmpX );
      _inVarsMap.insert( std::make_pair( inputVars[k], k) );
    }
    _X = IntVarArray( *this, tmpX );
    IntVarArgs tmpY;
    for ( unsigned k =0; k < outputVars.size(); k++ ) {
      processVariable( *this, info, outputVars[k], tmpY );
      _outVarsMap.insert( std::make_pair( outputVars[k], k ) );
    }
    _Y = IntVarArray( *this, tmpY );
  }

  SupportCSP::~SupportCSP() {

  }

  /* MRJ: See comment in support_csp.hxx
  void SupportCSP::addEqualityConstraint( VariableIdx v, int value ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    rel( *this, _X[ it->second ], IRT_EQ, value );
  }
  */

  void
  SupportCSP::addEqualityConstraint( VariableIdx v, bool value ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    value = ( value ? 1 : 0 );
    // MRJ: v = value
    rel( *this, _X[ it->second ], IRT_EQ, value );
  }

  void
  SupportCSP::addEqualityConstraint( VariableIdx v, ObjectIdx value ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    //MRJ: v = value
    rel( *this, _X[ it->second ], IRT_EQ, value );
  }


  void
  SupportCSP::addBoundsConstraint( VariableIdx v, int lb, int ub ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    // MRJ: lb <= v <= ub
    dom( *this, _X[ it->second], lb, ub);
  }

  void
  SupportCSP::addMembershipConstraint( VariableIdx v, DomainPtr varDomain ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    // MRJ: v \in dom
    TupleSet valueSet;
    for ( auto v : *varDomain ) {
      valueSet.add( IntArgs( 1, v ));
    }
    valueSet.finalize();
    // MRJ: v \in valueSet
    extensional( *this, IntVarArgs() << _X[ it->second ], valueSet );
  }

  SupportCSP::ptr
  SupportCSP::create( const ProblemInfo& info, const Action& a, const ScopedConstraint::vcptr& globalConstraints ) {
    // Determine input and output variables for this action
    VariableIdxSet inputVars, outputVars;
    // Add the variables mentioned by global constraints
    for ( ScopedConstraint::cptr global : globalConstraints )
      inputVars.insert( global.getScope().begin(), global.getScope().end() );
    // Add the variables mentioned in the preconditions
    for ( ScopedConstraint::cptr prec : a.getConstraints() )
      inputVars.insert( prec.getScope().begin(), prec.getScope().end() );
    // Add the variables appearing in the scope of the effects
    for ( ScopedEffect::cptr eff : a.getEffects() ) {}
      inputVars.insert( eff.getScope().begin(), eff.getScope().end() );
      outputVars.insert( eff.getAffected() );
    }
    VariableIdxVector tmpX( inputVars.begin(), inputVars.end() );
    VariableIdxVector tmpY( outputVars.begin(), outputVars.end() );
    SupportCSP::ptr csp = new SupportCSP( info, tmpX, tmpY );

    // Create constraints

  }

  void
  SupportCSP::create( const Problem& problem, SupportCSP::vptr& csps ) {
    const ProblemInfo& info = problem.getProblemInfo();
    for ( auto a : problem.getAllActions() )
      csps.push_back( SupportCSP::create( info, *a, problem.getConstraints() ) );
  }

}
