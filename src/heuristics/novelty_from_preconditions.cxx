#include <heuristics/novelty_from_preconditions.hxx>
#include <iostream>
#include <set>

namespace fs0 {

  NoveltyFromPreconditionsAdapter::NoveltyFromPreconditionsAdapter( const GenericState& s, const NoveltyFromPreconditions& featureMap )
		: _adapted( s ), _featureMap( featureMap) {}

	NoveltyFromPreconditionsAdapter::~NoveltyFromPreconditionsAdapter() { }

	NoveltyFromPreconditions::~NoveltyFromPreconditions() {
    for ( NoveltyFeature::ptr f : _features )
      delete f;
  }

  aptk::ValueIndex
  StateVarFeature::evaluate( const GenericState& s ) const {
    return s.getValue(_var);
  }


  aptk::ValueIndex
  ConstraintSetFeature::evaluate( const GenericState& s ) const {
    aptk::ValueIndex value = 0;
    for ( ScopedConstraint::cptr c : _constraints )
      if ( c->isSatisfied( s ) ) value++;
    return value;
  }

  void NoveltyFromPreconditions::selectFeatures( const Problem& theProblem, bool useGoal ) {

    std::set< VariableIdx > relevantVars;

    if ( useGoal ) {
      ConstraintSetFeature*  fG = new ConstraintSetFeature;
      for ( ScopedConstraint::cptr c : theProblem.getGoalConstraints() ) {
        fG->addConstraint(c);
        for ( VariableIdx x : c->getScope() )
          relevantVars.insert(x);
      }
      _features.push_back(fG);
    }

    for ( Action::cptr a : theProblem.getAllActions() ) {
      //ConstraintSetFeature*  f_a = new ConstraintSetFeature;
      for ( ScopedConstraint::cptr c : a->getConstraints() ) {
        // Leave out bounds constraints!
        if ( dynamic_cast< const UnaryDomainBoundsConstraint*>(c) != nullptr ) continue;
        if ( dynamic_cast< const BinaryDomainBoundsConstraint*>(c) != nullptr ) continue;
        for ( VariableIdx x : c->getScope() )
          relevantVars.insert(x);

        //f_a->addConstraint( c);
      }
      //_features.push_back(f_a);
    }

    for ( VariableIdx x : relevantVars ) {
      _features.push_back( new StateVarFeature( x ) );
    }

    std::cout << "Novelty From Constraints: # features: " << numFeatures() << std::endl;
  }

}
