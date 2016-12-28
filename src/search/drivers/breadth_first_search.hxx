
#pragma once

#include <search/drivers/registry.hxx>
#include <lapkt/nodes/blind_node.hxx>
#include <lapkt/algorithms/breadth_first_search.hxx>
#include <search/stats.hxx>
#include <state.hxx>
#include <actions/actions.hxx>

#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for an standard Breadth-First Search engine
class BreadthFirstSearchDriver : public Driver {
public:
	//! The Breadth-First Search engine uses a simple blind-search node
	using NodeT = lapkt::BlindSearchNode<State, GroundAction>;
	
	using Engine = std::unique_ptr<lapkt::StlBreadthFirstSearch<NodeT, GroundStateModel>>;
	
	Engine create(const Config& config, const GroundStateModel& model);
	
	GroundStateModel setup(Problem& problem) const;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

protected:
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	SearchStats _stats;
};

} } // namespaces
