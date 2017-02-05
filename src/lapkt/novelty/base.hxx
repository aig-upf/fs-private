
#pragma once

#include <vector>
#include <bits/stl_numeric.h>

namespace lapkt { namespace novelty {

//! Compute a vector with the indexes of those elements in a given valuation that are novel wrt a "parent" valuation.
template <typename FeatureValueT>
std::vector<unsigned> derive_novel(const std::vector<FeatureValueT>& current, const std::vector<FeatureValueT>& parent) {
	std::vector<unsigned> novel;
	for (unsigned i = 0; i < current.size(); ++i) {
		if (current[i] != parent[i]) {
			novel.push_back(i);
		}
	}
	return novel;
}

template <typename FeatureValueT>
class NoveltyEvaluatorI {
public:
	using ValuationT = std::vector<FeatureValueT>;
	
	virtual ~NoveltyEvaluatorI() = default;
	virtual NoveltyEvaluatorI* clone() const = 0;
	
	virtual bool evaluate_width_1_tuples(const ValuationT& valuation, const std::vector<unsigned>& novel) = 0;
	virtual bool evaluate_width_2_tuples(const ValuationT& valuation, const std::vector<unsigned>& novel) = 0;
	
	
	unsigned evaluate(const ValuationT& valuation) {
		setup_all_features_novel(valuation);
		return _evaluate(valuation, _all_features_novel);
	}
	
	//! Evaluate the novelty of a node taking into account the valuation of its parent, for optimization purposes
	unsigned evaluate(const ValuationT& valuation, const ValuationT& parent_valuation) {
		std::vector<unsigned> novel = derive_novel(valuation, parent_valuation);
		return _evaluate(valuation, novel);
	}
	
	unsigned evaluate(const ValuationT& valuation, unsigned k) {
		setup_all_features_novel(valuation);
		return _evaluate(valuation, _all_features_novel, k);
	}
	
	//! Evaluate the novelty of a node taking into account the valuation of its parent, for optimization purposes
	unsigned evaluate(const ValuationT& valuation, const ValuationT& parent_valuation, unsigned k) {
		std::vector<unsigned> novel = derive_novel(valuation, parent_valuation);
		return _evaluate(valuation, novel, k);
	}
	
protected:
	//! Evaluate the novelty of a given feature valuation, taking into account that only those indexes given in 'novel'
	//! contain values that can actually be novel.
	virtual unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel) = 0;
	
	//! Check only if the valuation contains a width-'k' tuple which is novel; return k if that is the case, or MAX if not
	virtual unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel, unsigned k) = 0;
	
protected:
	//! This is used to cache a vector <0,1,...,k> of appropriate length and spare the creation of one each time we need it.
	mutable std::vector<unsigned> _all_features_novel;	
	
	void setup_all_features_novel(const ValuationT& valuation) {
		std::size_t num_features = valuation.size();
		if (_all_features_novel.size() != num_features) {
			_all_features_novel.resize(num_features);
			std::iota(_all_features_novel.begin(), _all_features_novel.end(), 0);
		}		
	}
};

} } // namespaces
