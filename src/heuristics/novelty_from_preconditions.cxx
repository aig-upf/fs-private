
#include <iostream>
#include <set>

#include <heuristics/novelty_from_preconditions.hxx>
#include <languages/fstrips/scopes.hxx>
#include <utils/logging.hxx>
#include <utils/printers/feature_set.hxx>

namespace fs0 {

NoveltyFromPreconditionsAdapter::NoveltyFromPreconditionsAdapter( const State& s, const NoveltyFromPreconditions& featureMap )
	: _adapted( s ), _featureMap( featureMap) {}

NoveltyFromPreconditionsAdapter::~NoveltyFromPreconditionsAdapter() {}

NoveltyFromPreconditions::~NoveltyFromPreconditions() {
	for ( NoveltyFeature::ptr f : _features ) delete f;
}

aptk::ValueIndex StateVarFeature::evaluate( const State& s ) const { return s.getValue(_variable); }


aptk::ValueIndex ConstraintSetFeature::evaluate( const State& s ) const {
	aptk::ValueIndex value = 0;
	for ( AtomicFormula::cptr c : _conditions ) {
		if ( c->interpret( s ) ) value++;
	}
	return value;
}

void NoveltyFromPreconditions::selectFeatures( const Problem& problem, bool useStateVars, bool useGoal, bool useActions ) {
	std::set< VariableIdx > relevantVars;

	if ( useGoal ) {
		ConstraintSetFeature* feature = new ConstraintSetFeature;
		for ( AtomicFormula::cptr condition : problem.getGoalConditions() ) {
			feature->addCondition(condition);
			const auto scope = ScopeUtils::computeDirectScope(condition); // TODO - Should we also add the indirect scope?
			relevantVars.insert(scope.cbegin(), scope.cend());
		}
		_features.push_back(feature);
	}

	for ( GroundAction::cptr action : problem.getGroundActions() ) {
		ConstraintSetFeature*  feature = new ConstraintSetFeature;
		
		for ( AtomicFormula::cptr condition : action->getConditions() ) {
			if ( useStateVars ) {
				const auto scope = ScopeUtils::computeDirectScope(condition); // TODO - Should we also add the indirect scope?
				relevantVars.insert(scope.cbegin(), scope.cend());
			}
			if ( useActions ) feature->addCondition(condition);
		}
		
		if (useActions) _features.push_back(feature);
		else delete feature;
	}

	if ( useStateVars ) {
		for ( VariableIdx x : relevantVars ) {
			_features.push_back( new StateVarFeature( x ) );
		}
	}
	FINFO("main", "Novelty From Constraints: # features: " << numFeatures());
}
  
void NoveltyFromPreconditionsAdapter::get_valuation(std::vector<aptk::VariableIndex>& varnames, std::vector<aptk::ValueIndex>& values) const {
	if ( varnames.size() != _featureMap.numFeatures() ) {
		varnames.resize( _featureMap.numFeatures() );
	}
	
	if ( values.size() != _featureMap.numFeatures() ) {
		values.resize( _featureMap.numFeatures() );
	}
	
	for ( unsigned k = 0; k < _featureMap.numFeatures(); k++ ) {
		varnames[k] = k;
		values[k] = _featureMap.feature( k )->evaluate( _adapted );
	}
	
	FDEBUG("heuristic", "Feature evaluation: " << std::endl << print::feature_set(varnames, values));
}

} // namespaces
