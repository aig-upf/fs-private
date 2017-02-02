
#pragma once

#include <memory>
#include <cassert>

#include <lapkt/novelty/multivalued_evaluator.hxx>


namespace fs0 { namespace bfws {

/*
//! @deprecated, to be removed
class IWNoveltyEvaluator : public lapkt::novelty::MultivaluedNoveltyEvaluator<int> {
public:
	using Base = MultivaluedNoveltyEvaluator;

	IWNoveltyEvaluator(unsigned novelty_bound);
	~IWNoveltyEvaluator() = default;
	IWNoveltyEvaluator(const IWNoveltyEvaluator&) = default;
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	template <typename NodeT>
	unsigned evaluate(const NodeT& node, unsigned check_only_width = 0) {
		const lapkt::novelty::FeatureValuation& valuation = node.feature_valuation;
		assert(!valuation.empty());
		
		const lapkt::novelty::FeatureValuation* parent_valuation = nullptr;
		if (node.parent && node.parent->type() == node.type()) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			parent_valuation = &(node.parent->feature_valuation);
		}
		
		std::vector<unsigned> novel = derive_novel(node.feature_valuation, parent_valuation);
		return check_only_width > 0 ? evaluate(valuation, novel, check_only_width) : evaluate(valuation, novel);
	}
};
*/

class IWBinaryNoveltyEvaluator : public lapkt::novelty::MultivaluedNoveltyEvaluator<bool> {
public:
	using Base = MultivaluedNoveltyEvaluator;

	IWBinaryNoveltyEvaluator(unsigned novelty_bound) :
		Base(novelty_bound)
	{}
	
	~IWBinaryNoveltyEvaluator() = default;
	IWBinaryNoveltyEvaluator(const IWBinaryNoveltyEvaluator&) = default;
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	template <typename NodeT>
	unsigned evaluate(const NodeT& node, const lapkt::novelty::BinaryFeatureValuation& valuation, unsigned check_only_width = 0) {
		std::size_t num_features = valuation.size();
		if (_all_features_novel.size() != num_features) {
			_all_features_novel.resize(num_features);
			std::iota(_all_features_novel.begin(), _all_features_novel.end(), 0);
		}
		
		return check_only_width > 0 ? evaluate(valuation, _all_features_novel, check_only_width) : evaluate(valuation, _all_features_novel);
	}
	
	//! Evaluate the novelty of a node taking into account the valuation of its parent, for optimization purposes
	template <typename NodeT>
	unsigned evaluate(const NodeT& node, const lapkt::novelty::BinaryFeatureValuation& valuation, const lapkt::novelty::BinaryFeatureValuation& parent_valuation, unsigned check_only_width = 0) {
		std::vector<unsigned> novel = derive_novel(valuation, parent_valuation);
		return check_only_width > 0 ? evaluate(valuation, novel, check_only_width) : evaluate(valuation, novel);
	}
	
protected:
	//! This is used to cache a vector <0,1,...,k> of appropriate length and spare the creation of one each time we need it.
	mutable std::vector<unsigned> _all_features_novel;
};

} } // namespaces
