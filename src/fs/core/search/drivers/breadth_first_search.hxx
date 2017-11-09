
#pragma once

#include <vector>
#include <memory>

#include <lapkt/tools/events.hxx>

#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/stats.hxx>


namespace fs0 { class Config; }

namespace fs0 { namespace drivers {

//! A creator for an standard Breadth-First Search engine
template <typename StateModelT>
class BreadthFirstSearchDriver : public Driver {
public:

	StateModelT setup(Problem& problem) const;

	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

protected:
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	SearchStats _stats;
};

} } // namespaces
