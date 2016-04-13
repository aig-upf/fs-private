
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <search/components/single_novelty.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

//! A creator for a Breadth-First Search with Novelty pruning engine
class IteratedWidthDriver : public Driver {
public:
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const GroundStateModel& model) const;
};

} } // namespaces
