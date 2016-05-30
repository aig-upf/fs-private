
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for an standard Breadth-First Search engine
class BreadthFirstSearchDriver : public Driver {
public:
	//! The Breadth-First Search engine uses a simple blind-search node
	typedef BlindSearchNode<fs0::State> SearchNode;
	
	std::unique_ptr<FSGroundSearchAlgorithm> create(const Config& config, const GroundStateModel& model) const {
		FSGroundSearchAlgorithm* engine = new aptk::StlBreadthFirstSearch<SearchNode, GroundStateModel>(model);
		return std::unique_ptr<FSGroundSearchAlgorithm>(engine);
	}
	
};

} } // namespaces
