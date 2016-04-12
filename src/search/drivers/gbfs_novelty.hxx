
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/gbfs_novelty_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class FS0StateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for the Iterated Width Search FS0 engine
class GBFSNoveltyEngineCreator : public EngineCreator {
public:
	//! We use a GBFS heuristic search node
	typedef GBFSNoveltyNode<fs0::State> SearchNode;
	
	typedef UnsatGoalsNoveltyComponent<SearchNode> NoveltyHeuristic;
	
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const;
};

} } // namespaces
