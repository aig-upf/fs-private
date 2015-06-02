#ifndef SUPPORT_CSP
#define SUPPORT_CSP

#include <gecode/int.hxx>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <assert>

namespace fs0 {

  class SupportCSP : public Gecode::Space {
  public:

    typedef   std::map< VariableIdx, unsigned >     VariableMap;

    SupportCSP( const ProblemInfo& info, const VariableIdxVector& inputVars, const VariableIdxVector& outputVars );
    virtual ~SupportCSP();

    const Gecode::IntVar&  resolveX( VariableIdx varName ) const {
      return resolveVariableName( varName, _X, _inVarsMap );
    }

    const Gecode::IntVar&  resolveY( VariableIdx varName ) const {
      return resolveVariableName( varName, _Y, _outVarsMap );
    }

    void addMembershipConstraint( VariableIdx varName, DomainPtr values );
    void addBoundsConstraint( VariableIdx varName, int lb, int ub );

    // Cloning constructor
    SupportCSP( bool share, SupportCSP& other ) :
      Gecode::Space( share, other ) {
      _X.update( *this, share, other._X );
      _Y.update( *this, share, other._Y );
    }

    virtual Gecode::Space*
    copy( bool share ) {
      return new SupportCSP( share, *this );
    }

  protected:

    const Gecode::IntVar& resolveVariableName( VariableIdx varName, const IntVarArray& actualVars, const VariableMap& varMap ) const {
      auto it = varMap.find( varName );
      assert( it != _inVarsMap.end() );
      return actualVars[ it->second ];
    }

  protected:
    //! Input variables
    Gecode::IntVarArray _X;
    //! Output variables
    Gecode::IntVarArray _Y;
    //! Input variables mapping
    VariableMap _inVarsMap;
    //! Output variables mapping
    VariableMap _outVarsMap;
  };

}

#endif /* end of include guard: SUPPORT_CSP */
