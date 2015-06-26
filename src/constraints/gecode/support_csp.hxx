#ifndef SUPPORT_CSP
#define SUPPORT_CSP

#include <gecode/int.hh>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <cassert>
#include <vector>
#include <actions.hxx>
#include <atoms.hxx>

namespace fs0 {

  namespace gecode {

    class SupportCSP : public Gecode::Space {
    public:

      typedef   SupportCSP *                          ptr;
      typedef   std::vector< SupportCSP::ptr >        vptr;
      typedef   std::map< VariableIdx, unsigned >     VariableMap;

      SupportCSP( const ProblemInfo& info, const VariableIdxVector& inputVars, const VariableIdxVector& outputVars );
      virtual ~SupportCSP();

      //! Returns the handle to the Gecode variable corresponding to the
      //! planning state variable varName featured on the r.h.s. of
      //! an effect
      const Gecode::IntVar&  resolveX( VariableIdx varName ) const {
        return resolveVariableName( varName, _X, _inVarsMap );
      }

      //! Returns the handle to the Gecode variable corresponding to the
      //! affected planning state variable varName
      const Gecode::IntVar&  resolveY( VariableIdx varName ) const {
        return resolveVariableName( varName, _Y, _outVarsMap );
      }

      //! Retrieves possible valuations for a given variable varName
      void retrieveAtomsForAffectedVariable( VariableIdx varName, Atom::vctr& atoms ) const;

      //! Retrieve possible ranges of values for a possible variable
      //! varName
      void retrieveRangesForAffectedVariable( VariableIdx varName, Atom::vrange& ranges  ) const;

      void addEqualityConstraint( VariableIdx varName, bool value );

      // MRJ: This overload will be necessary as soon as int and ObjectIdx
      // cease to be the same thing
      //void addEqualityConstraint( VariableIdx varName, int  value );

      //! Adds constraint of the form $varName = value$ to the CSP
      void addEqualityConstraint( VariableIdx varName, ObjectIdx value );

      //! Adds constraint of the form $varName \in values$ to the CSP
      void addMembershipConstraint( VariableIdx varName, DomainPtr values );

      //! Adds constraint of the form $lb <= varName <= ub$ to the CSP
      void addBoundsConstraint( VariableIdx varName, int lb, int ub );

      //! Adds constraint of the form $min <= varName <= max$ to the CSP,
      //! where min and max are the minimum and maximum values defined for
      //! the type of varName.
      void addBoundsConstraintFromDomain( VariableIdx varName );

      //! Cloning constructor, required by Gecode
      SupportCSP( bool share, SupportCSP& other ) :
        Gecode::Space( share, other ), _info( other._info )  {
        _X.update( *this, share, other._X );
        _Y.update( *this, share, other._Y );
      }

      //! Shallow copy operator, see notes on search in Gecode to
      //! get an idea of what is being "actually" copied
      virtual Gecode::Space*
      copy( bool share ) {
        return new SupportCSP( share, *this );
      }

      bool  locallyConsistent() {
        status();
        return !failed();
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
