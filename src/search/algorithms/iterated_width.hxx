
#pragma once

#include <functional>

#include <search/nodes/blind_search_node.hxx>
#include <search/components/single_novelty.hxx>
#include <search/drivers/registry.hxx>
#include <ground_state_model.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>

#include <aptk2/search/algorithms/generic_search.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/search/components/unsorted_open_list_impl.hxx>
#include <aptk2/search/components/closed_list_impl.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { namespace drivers {

//! The original IW algorithm, adapted to FStrips
class FS0IWAlgorithm : public FS0SearchAlgorithm {
public:
	//! IW uses a simple blind-search node
	typedef BlindSearchNode<State> SearchNode;
	
	//! IW uses a single novelty component as the open list evaluator
	typedef SingleNoveltyComponent<SearchNode> SearchNoveltyEvaluator;
	
	//! IW uses an unsorted queue with a NoveltyEvaluator acceptor
	typedef aptk::StlUnsortedFIFO<SearchNode, SearchNoveltyEvaluator> OpenList;
	
	//! The base algorithm for IW is a simple Breadth-First Search
	typedef aptk::StlBreadthFirstSearch<SearchNode, GroundStateModel, OpenList> BaseAlgorithm;
	
	FS0IWAlgorithm(const GroundStateModel& model, unsigned initial_max_width, unsigned final_max_width, const NoveltyFeaturesConfiguration& feature_configuration);
	
	virtual ~FS0IWAlgorithm();
	
	virtual bool search(const State& state, typename FS0SearchAlgorithm::Plan& solution);
	
	void setup_base_algorithm(unsigned max_width);
	
protected:
	
	//!
	BaseAlgorithm* _algorithm;
	
	//!
	unsigned _current_max_width;
	
	//!
	unsigned _final_max_width;

	//! Novelty evaluator configuration
	const NoveltyFeaturesConfiguration _feature_configuration;
};

} } // namespaces
