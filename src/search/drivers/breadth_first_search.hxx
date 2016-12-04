
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <search/algorithms/aptk/breadth_first_search.hxx>
#include <search/stats.hxx>

#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for an standard Breadth-First Search engine
class BreadthFirstSearchDriver {
public:
	//! The Breadth-First Search engine uses a simple blind-search node
	using NodeT = BlindSearchNode<fs0::State>;
	
	using Engine = std::unique_ptr<lapkt::StlBreadthFirstSearch<NodeT, GroundStateModel>>;
	
	Engine create(const Config& config, const GroundStateModel& model);
	
	GroundStateModel setup(const Config& config, Problem& problem) const;
	
	SearchStats& getSearchStats() { return _stats; }

protected:
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	SearchStats _stats;
};

} } // namespaces
