
#include <iostream>
#include <set>

#include <heuristics/novelty/fs0_novelty_evaluator.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>
#include <languages/fstrips/scopes.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/feature_set.hxx>
#include <actions/actions.hxx>

namespace fs0 {

GenericStateAdapter::GenericStateAdapter( const State& s, const GenericNoveltyEvaluator& featureMap )
	: _adapted( s ), _featureMap( featureMap) {}

GenericStateAdapter::~GenericStateAdapter() {}

GenericNoveltyEvaluator::GenericNoveltyEvaluator(const Problem& problem, unsigned novelty_bound, const NoveltyFeaturesConfiguration& feature_configuration)
	: Base()
{
	set_max_novelty(novelty_bound);
	selectFeatures(problem, feature_configuration);
}


GenericNoveltyEvaluator::GenericNoveltyEvaluator(const GenericNoveltyEvaluator& other)
	: Base(other), _features() {
	for (unsigned i = 0; i < other._features.size(); ++i) {
		_features.push_back(std::unique_ptr<NoveltyFeature>(other._features[i]->clone()));
	}
}

void GenericNoveltyEvaluator::selectFeatures(const Problem& problem, const NoveltyFeaturesConfiguration& feature_configuration) {
	std::set< VariableIdx > relevantVars;

	if ( feature_configuration.useGoal() ) {
		ConditionSetFeature* feature = new ConditionSetFeature;
		for (const fs::AtomicFormula* condition : problem.getGoalConditions()->all_atoms() ) {
			feature->addCondition(condition);
			const auto scope = fs::ScopeUtils::computeDirectScope(condition); // TODO - Should we also add the indirect scope?
			relevantVars.insert(scope.cbegin(), scope.cend());
		}
		_features.push_back(std::unique_ptr<NoveltyFeature>(feature));
	}

	for ( const GroundAction* action : problem.getGroundActions() ) {
		std::unique_ptr<ConditionSetFeature> feature(new ConditionSetFeature);

		// TODO Need to rethink that to do with indirect scopes and in particular with existentially quantified variables
		for (const fs::AtomicFormula* condition : action->getPrecondition()->all_atoms() ) {
			if ( feature_configuration.useStateVars() ) {
				const auto scope = fs::ScopeUtils::computeDirectScope(condition); // TODO - Should we also add the indirect scope?
				relevantVars.insert(scope.cbegin(), scope.cend());
			}
			if ( feature_configuration.useActions() ) feature->addCondition(condition);
		}
		
		if (feature_configuration.useActions()) _features.push_back(std::move(feature));
	}

	if ( feature_configuration.useStateVars() ) {
		for ( VariableIdx var : relevantVars ) {
			_features.push_back(std::unique_ptr<NoveltyFeature>(new StateVariableFeature(var)));
		}
	}
	LPT_INFO("main", "Novelty From Constraints: # features: " << numFeatures());
}

void GenericStateAdapter::get_valuation(std::vector<aptk::VariableIndex>& varnames, std::vector<aptk::ValueIndex>& values) const {
	if ( varnames.size() != _featureMap.numFeatures() ) {
		varnames.resize( _featureMap.numFeatures() );
	}

	if ( values.size() != _featureMap.numFeatures() ) {
		values.resize( _featureMap.numFeatures() );
	}

	for ( unsigned k = 0; k < _featureMap.numFeatures(); k++ ) {
		varnames[k] = k;
		values[k] = _featureMap.feature(k).evaluate( _adapted );
	}

	LPT_DEBUG("heuristic", "Feature evaluation: " << std::endl << print::feature_set(varnames, values));
}

} // namespaces
