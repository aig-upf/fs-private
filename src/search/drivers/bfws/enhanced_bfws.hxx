
#pragma once

#include <memory>

#include <search/drivers/bfws/bfws_base.hxx>
#include <search/drivers/registry.hxx>
#include <search/drivers/smart_effect_driver.hxx>
#include <search/nodes/bfws_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <search/stats.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <heuristics/novelty/multivalued_evaluator.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>


#include <search/algorithms/aptk/sorted_open_list.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>

using OffendingSet = std::unordered_set<fs0::ObjectIdx>;

namespace fs0 { class Config; }

namespace fs0 { namespace drivers {

class EnhancedBFWSDriver : public Driver {
public:
	using ActionT = GroundAction;
	using NodeT = BFWSNode<fs0::State, ActionT>;
	using HeuristicT = UnsatGoalsNoveltyComponent<GroundStateModel, NodeT>;
	using Engine = std::unique_ptr<lapkt::StlBestFirstSearch<NodeT, HeuristicT, GroundStateModel>>;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
	
protected:
	
	std::vector<OffendingSet> preprocess(const Problem& problem, const Config& config, BasicApplicabilityAnalyzer* analyzer);

	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	//! A partial specialization
	template <typename NodeCompareT>
	ExitCode do_search(Problem& problem, const Config& config, const std::string& out_dir, float start_time);
	
	template <typename NodeCompareT, typename NoveltyIndexerT>
	ExitCode do_search(Problem& problem, const Config& config, const std::string& out_dir, float start_time);
	
	template <typename NodeCompareT, typename NoveltyIndexerT, typename PrunerT, typename ClosedListT>
	ExitCode do_search_1(Problem& problem, const Config& config, const std::string& out_dir, float start_time);
	
};



class CTMPNoveltyEvaluator : public MultivaluedNoveltyEvaluator {
public:
	using Base = MultivaluedNoveltyEvaluator;
	using FeatureSet = std::vector<std::unique_ptr<NoveltyFeature>>;

	CTMPNoveltyEvaluator(const Problem& problem, unsigned novelty_bound, const NoveltyFeaturesConfiguration& feature_configuration, bool check_overlaps=false, bool placeable = true, bool graspable = false);
	virtual ~CTMPNoveltyEvaluator() = default;
	CTMPNoveltyEvaluator(const CTMPNoveltyEvaluator&);
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	template <typename NodeT>
	unsigned evaluate(const NodeT& node) {
		const FeatureValuation& valuation = node.feature_valuation;
		assert(!valuation.empty());
		
		const FeatureValuation* parent_valuation = nullptr;
		if (node.parent && node.parent->type == node.type) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			parent_valuation = &(node.parent->feature_valuation);
		}
		
		std::vector<unsigned> novel = derive_novel(node.feature_valuation, parent_valuation);
		return evaluate(valuation, novel);
	}
	
	//! Compute the feature valuation of a given state
	std::vector<ValuesTuple::ValueIndex> compute_valuation(const State& state) const;

protected:
	//! Select and create the state features that we will use henceforth to compute the novelty
	static FeatureSet selectFeatures(const Problem& problem, const NoveltyFeaturesConfiguration& config, bool check_overlaps, bool placeable, bool graspable);
	
	static FeatureSet clone_features(const FeatureSet& features);
	
	//! An array with all the features that we take into account when computing the novelty
	const FeatureSet _features;
};

template <typename StateModelT,
          typename SearchNode>
class CTMPOpenListAcceptor {
protected:
	unsigned _bound;
	
	//! A single novelty evaluator will be in charge of evaluating all nodes
	CTMPNoveltyEvaluator _novelty_evaluator;

public:
	
	CTMPOpenListAcceptor(const StateModelT& model, unsigned max_novelty, const NoveltyFeaturesConfiguration& feature_configuration)
		: _bound(max_novelty), _novelty_evaluator(model.getTask(), max_novelty, feature_configuration)
	{}
	
	~CTMPOpenListAcceptor() = default;

	//! Returns false iff we want to prune this node during the search
	bool accept(SearchNode& node) {
		assert(node.feature_valuation.empty()); // We should not be computing the feature valuation twice
		node.feature_valuation = _novelty_evaluator.compute_valuation(node.state);
		return _novelty_evaluator.evaluate(node) <= _bound;
	}
};


} } // namespaces



