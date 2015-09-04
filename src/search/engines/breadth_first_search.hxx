
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <search/components/novelty_evaluator.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class FS0StateModel; class Config; }

namespace fs0 { namespace engines {

//! A creator for an standard Breadth-First Search engine
class BreadthFirstSearchEngineCreator : public EngineCreator {
public:
	//! The Breadth-First Search engine uses a simple blind-search node
	typedef BlindSearchNode<fs0::State> SearchNode;
	
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const {
		FS0SearchAlgorithm* engine = new aptk::StlBreadthFirstSearch<SearchNode, FS0StateModel>(model);
		return std::unique_ptr<FS0SearchAlgorithm>(engine);
	}
};

} } // namespaces
