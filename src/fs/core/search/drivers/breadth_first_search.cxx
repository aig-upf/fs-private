
#include <fs/core/search/drivers/breadth_first_search.hxx>
#include <fs/core/state.hxx>
#include <fs/core/search/nodes/blind_node.hxx>
#include <lapkt/algorithms/breadth_first_search.hxx>
#include <fs/core/search/events.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/search/drivers/setups.hxx>


namespace fs0 { namespace drivers {

template <>
GroundStateModel
BreadthFirstSearchDriver<GroundStateModel>::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}

template <>
CSPLiftedStateModel
BreadthFirstSearchDriver<CSPLiftedStateModel>::setup(Problem& problem) const {
	return GroundingSetup::fully_lifted_model(problem);
}

template <>
SDDLiftedStateModel
BreadthFirstSearchDriver<SDDLiftedStateModel>::setup(Problem& problem) const {
    return GroundingSetup::sdd_lifted_model(problem);
}

template <typename StateModelT>
ExitCode
BreadthFirstSearchDriver<StateModelT>::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	//! The Breadth-First Search engine uses a simple blind-search node
	using ActionT = typename StateModelT::ActionType;
	using NodeT = lapkt::BlindSearchNode<State, ActionT>;
	using EngineT = lapkt::StlBreadthFirstSearch<NodeT, StateModelT>;
	using EnginePT = std::unique_ptr<EngineT>;

	auto model = setup(problem);

    EventUtils::setup_stats_observer<NodeT>(_stats, _handlers, config.getOption<bool>("verbose_stats", false));
	auto engine = EnginePT(new EngineT(model));
	lapkt::events::subscribe(*engine, _handlers);
	
	return Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, _stats);
}

// explicit instantiations
template class BreadthFirstSearchDriver<GroundStateModel>;
template class BreadthFirstSearchDriver<CSPLiftedStateModel>;
template class BreadthFirstSearchDriver<SDDLiftedStateModel>;

} } // namespaces
