
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <search/components/novelty_evaluator.hxx>
#include <search/components/novelty_reachability_ensemble_evaluator.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class FS0StateModel; class Config; }

namespace fs0 { namespace engines {

//! A creator for the Iterated Width Search FS0 engine
class IWGBFSEngineCreator : public EngineCreator {
public:
	//! We use a GBFS heuristic search node
	typedef HeuristicSearchNode<fs0::State> SearchNode;
	
	typedef NoveltyReachabilityEnsembleEvaluator<SearchNode> NoveltyHeuristic;
	
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const;
};

} } // namespaces
