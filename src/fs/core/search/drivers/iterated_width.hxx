
#pragma once

#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/stats.hxx>
#include <fs/core/search/algorithms/iterated_width.hxx>

namespace fs0 { class Config; }

namespace fs0 { namespace drivers {

//! A creator for an IW algorithm
template <typename StateModelT>
class IteratedWidthDriver : public Driver {
public:
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
protected:
	SearchStats _stats;
};


} } // namespaces
