
#pragma once

#include <search/drivers/registry.hxx>
#include <search/components/single_novelty.hxx>
#include <lapkt/events.hxx>
#include <search/algorithms/iterated_width.hxx>
#include <search/stats.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class Config; }

namespace fs0 { namespace drivers {

//! A creator for an IW algorithm
template <typename StateModelT>
class IteratedWidthDriver : public Driver {
public:
	using Engine = FS0IWAlgorithm<StateModelT>;
	using EnginePtr = std::unique_ptr<Engine>;
	
	EnginePtr create(const Config& config, const StateModelT& model);
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
protected:
	SearchStats _stats;
};


} } // namespaces
