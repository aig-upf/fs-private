
#pragma once

#include <memory>
#include <cassert>

#include <heuristics/novelty/multivalued_evaluator.hxx>
#include <heuristics/novelty/features.hxx>


namespace fs0 { namespace bfws {

using FeatureSet = std::vector<std::unique_ptr<fs0::NoveltyFeature>>;

class IWNoveltyEvaluator : public fs0::MultivaluedNoveltyEvaluator {
public:
	using Base = MultivaluedNoveltyEvaluator;

	IWNoveltyEvaluator(unsigned novelty_bound, const FeatureSet& features);
	~IWNoveltyEvaluator() = default;
	IWNoveltyEvaluator(const IWNoveltyEvaluator&) = default;
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	template <typename NodeT>
	unsigned evaluate(const NodeT& node) {
		const FeatureValuation& valuation = node.feature_valuation;
		assert(!valuation.empty());
		
		const FeatureValuation* parent_valuation = nullptr;
		if (node.parent && node.parent->type() == node.type()) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			parent_valuation = &(node.parent->feature_valuation);
		}
		
		std::vector<unsigned> novel = derive_novel(node.feature_valuation, parent_valuation);
		return evaluate(valuation, novel);
	}
	
	//! Compute the feature valuation of a given state
	std::vector<ValuesTuple::ValueIndex> compute_valuation(const State& state) const;

protected:
	//! An array with all the features that we take into account when computing the novelty
	const FeatureSet& _features;
};

} } // namespaces
