
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <search/components/single_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/stats.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for a Breadth-First Search with Novelty pruning engine
class IteratedWidthDriver : public Driver {
public:
	std::unique_ptr<FSGroundSearchAlgorithm> create(const Config& config, const GroundStateModel& model);
	
	GroundStateModel setup(Problem& problem) const;

	void search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

	
protected:
	SearchStats _stats;

};

} } // namespaces
