
#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <heuristics/novelty/features.hxx>
#include <state.hxx>
#include <problem.hxx>

namespace fs0 {

class GenericNoveltyEvaluator;

class GenericStateAdapter {
public:
	GenericStateAdapter( const State& s, const GenericNoveltyEvaluator& featureMap );
	~GenericStateAdapter();

	void get_valuation( std::vector< aptk::VariableIndex >& varnames, std::vector< aptk::ValueIndex >& values ) const;

protected:
	const State& _adapted;
	const GenericNoveltyEvaluator& _featureMap;
};



class GenericNoveltyEvaluator : public aptk::FiniteDomainGenericNoveltyEvaluator< GenericStateAdapter > {
public:
	typedef aptk::FiniteDomainGenericNoveltyEvaluator< GenericStateAdapter > BaseClass;

	void selectFeatures( const Problem& problem, bool useStateVars, bool useGoal, bool useActions );

	unsigned evaluate( const State& s ) {
		GenericStateAdapter adaptee( s, *this );
		return BaseClass::evaluate( adaptee );
	}

	unsigned numFeatures() const { return _features.size(); }
	NoveltyFeature::ptr feature( unsigned i ) const { return _features[i]; }

	virtual ~GenericNoveltyEvaluator();

protected:
	//!
	std::vector<NoveltyFeature::ptr> _features;
};


}
