#ifndef __NOVELTY_FROM_PRECONDITIONS__
#define __NOVELTY_FROM_PRECONDITIONS__

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <state.hxx>
#include <constraints/scoped_constraint.hxx>
#include <problem.hxx>

namespace fs0 {

  class NoveltyFromPreconditions;

  class NoveltyFromPreconditionsAdapter {
  public:
    NoveltyFromPreconditionsAdapter( const GenericState& s, const NoveltyFromPreconditions& featureMap );
    ~NoveltyFromPreconditionsAdapter();

    void	get_valuation( 	std::vector< aptk::VariableIndex >& varnames,
          std::vector< aptk::ValueIndex >& values ) const;
  protected:
    const GenericState&                 _adapted;
    const NoveltyFromPreconditions&     _featureMap;
  };

  class NoveltyFeature {
  public:
    typedef NoveltyFeature *                     ptr;
    typedef std::vector< NoveltyFeature::ptr >   vptr;


    virtual ~NoveltyFeature() {}
    virtual aptk::ValueIndex  evaluate( const GenericState& s ) const = 0;

  };

  class StateVarFeature : public NoveltyFeature {
  public:

    StateVarFeature( VariableIdx x ) : _var(x) {}
    ~StateVarFeature() {}
    aptk::ValueIndex  evaluate( const GenericState& s ) const;

  protected:
    VariableIdx     _var;

  };

  class ConstraintSetFeature : public NoveltyFeature {
  public:

    ConstraintSetFeature() {}
    ~ConstraintSetFeature() {}
    void addConstraint( ScopedConstraint::cptr c) {
      _constraints.push_back(c);
    }

    aptk::ValueIndex  evaluate( const GenericState& s ) const;

  protected:
    ScopedConstraint::vcptr   _constraints;
  };


  class NoveltyFromPreconditions : public aptk::FiniteDomainNoveltyEvaluator< NoveltyFromPreconditionsAdapter > {
	public:
		typedef aptk::FiniteDomainNoveltyEvaluator< NoveltyFromPreconditionsAdapter > BaseClass;


    void  selectFeatures( const Problem& problem, bool useStateVars, bool useGoal, bool useActions );

		unsigned evaluate( const GenericState& s ) {
      NoveltyFromPreconditionsAdapter adaptee( s, *this );
			return BaseClass::evaluate( adaptee );
		}

    unsigned                  numFeatures() const { return _features.size(); }
    NoveltyFeature::ptr       feature( unsigned i ) const { return _features[i]; }

		virtual ~NoveltyFromPreconditions();

  private:
    NoveltyFeature::vptr         _features;
	};

  inline void
  NoveltyFromPreconditionsAdapter::get_valuation( std::vector< aptk::VariableIndex >& varnames,
                                                std::vector< aptk::ValueIndex >& values ) const {

   if ( varnames.size() != _featureMap.numFeatures() )
     varnames.resize( _featureMap.numFeatures() );
   if ( values.size() != _featureMap.numFeatures() )
     values.resize( _featureMap.numFeatures() );
   for ( unsigned k = 0; k < _featureMap.numFeatures(); k++ ) {
     varnames[k] = k;
     values[k] = _featureMap.feature( k )->evaluate( _adapted );
   }
   #ifdef DEBUG
   std::cout << "Features evaluation :(";
   for ( unsigned i = 0; i < values.size(); i++ ) {
     std::cout << varnames[i] << " = " << values[i] << ", ";
   }

   std::cout << ")" << std::endl;
   #endif
 }


}

#endif // novelty_from_preconditions.hxx
