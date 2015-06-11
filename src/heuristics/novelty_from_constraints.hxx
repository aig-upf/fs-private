#ifndef __NOVELTY_FROM_CONSTRAINTS__
#define __NOVELTY_FROM_CONSTRAINTS__

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <state.hxx>
#include <constraints/scoped_constraint.hxx>
#include <problem.hxx>

namespace fs0 {

  class NoveltyFromConstraints;

  class NoveltyFromConstraintsAdapter {
  public:
    NoveltyFromConstraintsAdapter( const GenericState& s, const NoveltyFromConstraints& featureMap );
    ~NoveltyFromConstraintsAdapter();

    void	get_valuation( 	std::vector< aptk::VariableIndex >& varnames,
          std::vector< aptk::ValueIndex >& values ) const;
  protected:
    const GenericState&               _adapted;
    const NoveltyFromConstraints&     _featureMap;
  };


  class NoveltyFromConstraints : public aptk::FiniteDomainNoveltyEvaluator< NoveltyFromConstraintsAdapter > {
	public:
		typedef aptk::FiniteDomainNoveltyEvaluator< NoveltyFromConstraintsAdapter > BaseClass;


    void  selectFeatures( const Problem& problem, bool useGoal );

		unsigned evaluate( const GenericState& s ) {
      NoveltyFromConstraintsAdapter adaptee( s, *this );
			return BaseClass::evaluate( adaptee );
		}

    unsigned                    numFeatures() const { return _features.size(); }
    ScopedConstraint::cptr      feature( unsigned i ) const { return _features[i]; }

		virtual ~NoveltyFromConstraints();

  private:
    ScopedConstraint::vcptr       _features;
	};

  inline void
  NoveltyFromConstraintsAdapter::get_valuation( std::vector< aptk::VariableIndex >& varnames,
                                                std::vector< aptk::ValueIndex >& values ) const {

   if ( varnames.size() != _featureMap.numFeatures() )
     varnames.resize( _featureMap.numFeatures() );
   if ( values.size() != _featureMap.numFeatures() )
     values.resize( _featureMap.numFeatures() );
   for ( unsigned k = 0; k < _featureMap.numFeatures(); k++ ) {
     varnames[k] = k;
     values[k] = ( _featureMap.feature( k )->isSatisfied( _adapted ) ? 1 : 0 );
   }
 }


}

#endif // novelty_from_constraints.hxx
