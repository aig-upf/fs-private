
#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <heuristics/novelty/features.hxx>
#include <state.hxx>
#include <problem.hxx>

namespace fs0 {

class NoveltyFeaturesConfiguration;

class GenericNoveltyEvaluator;

using FSNoveltyFeature = lapkt::novelty::NoveltyFeature<State>;

class GenericStateAdapter {
public:
	GenericStateAdapter( const State& s, const GenericNoveltyEvaluator& featureMap );
	~GenericStateAdapter();

	void get_valuation( std::vector< aptk::VariableIndex >& varnames, std::vector< aptk::ValueIndex >& values ) const;

protected:
	const State& _adapted;
	const GenericNoveltyEvaluator& _featureMap;
};


class GenericNoveltyEvaluator : public aptk::FiniteDomainNoveltyEvaluator<GenericStateAdapter> {
public:
	using Base = aptk::FiniteDomainNoveltyEvaluator<GenericStateAdapter>;
	using FeatureSet = std::vector<std::unique_ptr<FSNoveltyFeature>>;
	

	GenericNoveltyEvaluator(const Problem& problem, unsigned novelty_bound, const NoveltyFeaturesConfiguration& feature_configuration);
	virtual ~GenericNoveltyEvaluator() = default;
	GenericNoveltyEvaluator(const GenericNoveltyEvaluator&);
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	unsigned evaluate(const State& s) {
		GenericStateAdapter adaptee(s, *this);
		return evaluate(adaptee);
	}

	unsigned numFeatures() const { return _features.size(); }
	FSNoveltyFeature& feature(unsigned i) const { return *_features[i]; }


protected:
	//! Select and create the state features that we will use henceforth to compute the novelty
	void selectFeatures(const Problem& problem, const NoveltyFeaturesConfiguration& feature_configuration);
	
	//! An array with all the features that we take into account when computing the novelty
	
	FeatureSet _features;
};


}
