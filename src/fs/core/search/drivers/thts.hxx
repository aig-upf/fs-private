
#pragma once

#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/stats.hxx>

namespace fs0 { class Config; }

namespace fs0 { namespace drivers {

template <typename StateModelT>
class THTSDriver : public Driver {
public:
	ExitCode search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) override;
	
protected:
	SearchStats _stats;
};


} } // namespaces
