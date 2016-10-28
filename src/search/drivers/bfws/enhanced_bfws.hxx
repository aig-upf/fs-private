
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
	
	std::vector<OffendingSet> preprocess(const Problem& problem, const Config& config);

	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
};



class CTMPNoveltyEvaluator;
class CTMPStateAdapter {
public:
	CTMPStateAdapter( const State& s, const CTMPNoveltyEvaluator& featureMap );
	~CTMPStateAdapter() = default;
	void get_valuation(std::vector<aptk::VariableIndex>& varnames, std::vector<aptk::ValueIndex>& values) const;

protected:
	const State& _adapted;
	const CTMPNoveltyEvaluator& _featureMap;
};


class CTMPNoveltyEvaluator : public aptk::FiniteDomainNoveltyEvaluator<CTMPStateAdapter> {
public:
	using Base = aptk::FiniteDomainNoveltyEvaluator<CTMPStateAdapter>;
	using FeatureSet = std::vector<std::unique_ptr<NoveltyFeature>>;
	

	CTMPNoveltyEvaluator(const Problem& problem, unsigned novelty_bound, const NoveltyFeaturesConfiguration& feature_configuration, bool check_overlaps=false);
	virtual ~CTMPNoveltyEvaluator() = default;
	CTMPNoveltyEvaluator(const CTMPNoveltyEvaluator&);
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	unsigned evaluate(const State& s) {
		CTMPStateAdapter adaptee(s, *this);
		return evaluate(adaptee);
	}

	unsigned numFeatures() const { return _features.size(); }
	NoveltyFeature& feature(unsigned i) const { return *_features[i]; }


protected:
	//! Select and create the state features that we will use henceforth to compute the novelty
	void selectFeatures(const Problem& problem, const NoveltyFeaturesConfiguration& config, bool check_overlaps);
	
	//! An array with all the features that we take into account when computing the novelty
	FeatureSet _features;
};



} } // namespaces



