
#pragma once

#include <memory>
#include <cassert>

#include <lapkt/novelty/multivalued_evaluator.hxx>


namespace fs0 { namespace bfws {


class IWNoveltyEvaluator : public lapkt::novelty::MultivaluedNoveltyEvaluator {
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

} } // namespaces
