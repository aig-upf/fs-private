//
// Created by Miquel Ramirez on 11/12/2019.
//
#include <fs/core/search/drivers/iterative_deepening_driver.hxx>
#include <fs/core/state.hxx>
#include <fs/core/search/nodes/id_node.hxx>
#include <fs/core/search/algorithms/iterative_deepening.hxx>
#include <fs/core/search/events.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <stdexcept>

namespace fs0::drivers {

    template<>
    GroundStateModel
    IterativeDeepeningDriver<GroundStateModel>::setup(Problem &problem) const {
        throw std::logic_error("IterativeDeepeningDriver: Ground state model is NOT supported!");
        return GroundingSetup::fully_ground_model(problem);
    }

    template<>
    LiftedStateModel
    IterativeDeepeningDriver<LiftedStateModel>::setup(Problem &problem) const {
        throw std::logic_error("IterativeDeepeningDriver: Lifted state model is NOT supported!");

        return GroundingSetup::fully_lifted_model(problem);
    }

    template<>
    SDDLiftedStateModel
    IterativeDeepeningDriver<SDDLiftedStateModel>::setup(Problem &problem) const {
        return GroundingSetup::sdd_lifted_model(problem);
    }

    template<typename StateModelT>
    ExitCode
    IterativeDeepeningDriver<StateModelT>::search(Problem &problem, const Config &config, const EngineOptions &options,
                                                  float start_time) {
        //! The Breadth-First Search engine uses a simple blind-search node
        using ActionT = typename StateModelT::ActionType;
        using NodeT = lapkt::IterativeDeepeningNode<State, ActionT, SDDActionIterator::Iterator>;
        using EngineT = lapkt::IterativeDeepening<NodeT, StateModelT>;
        using EnginePT = std::unique_ptr<EngineT>;

        auto model = setup(problem);

        EventUtils::setup_stats_observer<NodeT>(_stats, _handlers, config.getOption<bool>("verbose_stats", false));
        auto engine = EnginePT(new EngineT(model));
        lapkt::events::subscribe(*engine, _handlers);

        return Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, _stats);
    }

// explicit instantiations
    //template
    //class IterativeDeepeningDriver<GroundStateModel>;

    //template
    //class IterativeDeepeningDriver<LiftedStateModel>;

    template
    class IterativeDeepeningDriver<SDDLiftedStateModel>;

}// namespaces
