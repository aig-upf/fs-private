//
// Created by Miquel Ramirez on 11/12/2019.
//

#pragma once

#include <vector>
#include <memory>

#include <lapkt/tools/events.hxx>

#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/stats.hxx>


namespace fs0 { class Config; }

namespace fs0::drivers {

    //! A creator for an Iterative Deepening engine
    template <typename StateModelT>
    class IterativeDeepeningDriver : public Driver {
    public:

        StateModelT setup(Problem& problem) const;

        ExitCode search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) override;

    protected:
        std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;

        SearchStats _stats;
    };

} // namespaces
