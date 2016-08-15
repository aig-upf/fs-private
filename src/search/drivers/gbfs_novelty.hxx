
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/gbfs_novelty_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <search/stats.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for the Iterated Width Search FS0 engine
class GBFSNoveltyDriver {
public:
	//! We use a GBFS heuristic search node
	using NodeT = GBFSNoveltyNode<fs0::State>;
	
	using NoveltyHeuristic = UnsatGoalsNoveltyComponent<NodeT>;
	
	using Engine = std::unique_ptr<lapkt::StlBestFirstSearch<NodeT, NoveltyHeuristic, GroundStateModel>>;
	
	Engine create(const Config& config, const GroundStateModel& model);
	
	SearchStats& getSearchStats() { return _stats; }
	
	GroundStateModel setup(const Config& config, Problem& problem) const;

protected:
	std::unique_ptr<NoveltyHeuristic> _heuristic;
	
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	SearchStats _stats;
};

} } // namespaces
