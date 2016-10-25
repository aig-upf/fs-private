
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



#include <search/algorithms/aptk/generic_search.hxx>
#include <search/algorithms/aptk/sorted_open_list.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>
#include <aptk2/search/components/unsorted_open_list_impl.hxx>

using OffendingSet = std::unordered_set<fs0::ObjectIdx>;

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

template <typename StateModelT>
class EnhancedBFWSDriver : public Driver {
public:
	using ActionT = GroundAction;
	using NodeT = BFWSNode<fs0::State, ActionT>;
	using HeuristicT = UnsatGoalsNoveltyComponent<StateModelT, NodeT>;
	using Engine = std::unique_ptr<lapkt::StlBestFirstSearch<NodeT, HeuristicT, StateModelT>>;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
	
protected:
// 	Engine create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const StateModelT& model, SearchStats& stats);
	
	std::vector<OffendingSet> preprocess(const Problem& problem, const Config& config);

	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
};

} } // namespaces



