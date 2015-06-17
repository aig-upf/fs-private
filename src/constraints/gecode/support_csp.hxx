#ifndef SUPPORT_CSP
#define SUPPORT_CSP

#include <gecode/int.hh>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <cassert>
#include <vector>
#include <actions.hxx>

namespace fs0 {

  namespace gecode {

    class SupportCSP : public Gecode::Space {
    public:

      typedef   SupportCSP *                          ptr;
      typedef   std::vector< SupportCSP::ptr >        vptr;
      typedef   std::map< VariableIdx, unsigned >     VariableMap;

      SupportCSP( const ProblemInfo& info, const VariableIdxVector& inputVars, const VariableIdxVector& outputVars );
      virtual ~SupportCSP();

      const Gecode::IntVar&  resolveX( VariableIdx varName ) const {
        return resolveVariableName( varName, _X, _inVarsMap );
      }

      const Gecode::IntVar&  resolveY( VariableIdx varName ) const {
        return resolveVariableName( varName, _Y, _outVarsMap );
      }

      void addEqualityConstraint( VariableIdx varName, bool value );
      // MRJ: This overload will be necessary as soon as int and ObjectIdx
      // cease to be the same thing
      //void addEqualityConstraint( VariableIdx varName, int  value );
      void addEqualityConstraint( VariableIdx varName, ObjectIdx value );
      void addMembershipConstraint( VariableIdx varName, DomainPtr values );
      void addBoundsConstraint( VariableIdx varName, int lb, int ub );
      void addBoundsConstraintFromDomain( VariableIdx varName );

      // Cloning constructor
      SupportCSP( bool share, SupportCSP& other ) :
        Gecode::Space( share, other ), _info( other._info )  {
        _X.update( *this, share, other._X );
        _Y.update( *this, share, other._Y );
      }

      virtual Gecode::Space*
      copy( bool share ) {
        return new SupportCSP( share, *this );
      }

      static SupportCSP::ptr  create( const ProblemInfo& info, const Action& a, const ScopedConstraint::vcptr& globalConstraints );
      static void             create( const Problem& problem, SupportCSP::vptr& supportCSPs );

    protected:

      const Gecode::IntVar& resolveVariableName( VariableIdx varName, const Gecode::IntVarArray& actualVars, const VariableMap& varMap ) const {
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
      //! Reference to problem info
      const ProblemInfo&  _info;
    };


  }

}

#endif /* end of include guard: SUPPORT_CSP */
