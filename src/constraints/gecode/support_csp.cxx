#include <constraints/gecode/support_csp.hxx>

namespace fs0 {

  static void
  processVariable( const ProblemInfo& info, VariableIdx var, Gecode::IntVarArgs& varArray ) {
      ProblemInfo::ObjectType varType = info.getVariableType( var );
      if ( varType == ProblemInfo::ObjecType::INT ) {
        auto bounds = info.getVariableBounds(var);
        varArray << Gecode::IntVar( *this, bounds.first, bounds.second );
      }
      else if ( varType == ProblemInfo::ObjectType::BOOL ) {
        varArray << Gecode::IntVar( *this, 0, 1 );
      }
      else if ( varType == ProblemInfo::ObjecType::OBJECT ) {
        const ObjectIdxVector& values = info.getVariableObjects( var );
        int _values[ values.size() ];
        for ( unsigned j = 0; j < values.size(); j++ )
          _values[j] = values[j];
        varArray << Gecode::IntVar( *this, Gecode::IntSet( _values, values.size() ));
      }
  }

  SupportCSP::SupportCSP( const ProblemInfo& info, const VariableIdxVector& inputVars, const VariableIdxVector& outputVars) {
    Gecode::IntVarArgs tmpX;
    for ( unsigned k = 0; k < inputVars.size(); k++ ) {
      processVariable( info, var, tmpX );
      _inVarsMap.insert( std::make_pair( inputVars[k], k) );
    }
    _X = Gecode::IntVarArray( *this, tmpX );
    IntVarArgs tmpY;
    for ( unsigned k =0; k < outputVars.size(); k++ ) {
      processVariable( info, var, tmpY );
      _outVarsMap.insert( std::make_pair( outputVars[k], k ) );
    }
    _Y = Gecode::IntVarArray( *this, tmpY );
  }

  SupportCSP::~SupportCSP() {

  }

  void SupportCSP::addBoundsConstraint( VariableIdx v, int lb, int ub ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    // MRJ: lb <= v <= ub
    Gecode::dom( *this, _X[ it->second], lb, ub);
  }

  void SupportCSP::addMembershipConstraint( VariableIdx v, DomainPtr dom ) {
    auto it = _inVarsMap.find( v );
    assert( it != _inVarsMap.end() );
    // MRJ: v \in dom
    Gecode::TupleSet valueSet;
    for ( auto v : *dom ) {
      valueSet.add( Gecode::IntArs( 1, v ));
    }
    valueSet.finalize();
    Gecode::extensional( *this, _X[ it->second ], dom );
  }

}
