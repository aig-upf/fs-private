
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <search/components/single_novelty.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class FS0StateModel; class Config; }

namespace fs0 { namespace engines {

//! A creator for a Breadth-First Search with Novelty pruning engine
class IteratedWidthEngineCreator : public EngineCreator {
public:
	//! IW uses a simple blind-search node
	typedef BlindSearchNode<fs0::State> SearchNode;
	
	typedef SingleNoveltyComponent<SearchNode> SearchNoveltyEvaluator;
	
	//! IW uses an unsorted queue with a NoveltyEvaluator acceptor
	typedef aptk::StlUnsortedFIFO<SearchNode, SearchNoveltyEvaluator> OpenList;
	
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const;
};

} } // namespaces
