
#include <fs/core/search/drivers/breadth_first_search.hxx>

#include <fs/core/state.hxx>
#include <fs/core/search/nodes/blind_node.hxx>
#include <fs/core/search/algorithms/breadth_first_search.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/search/drivers/setups.hxx>


namespace fs0::drivers {

template <>
GroundStateModel
BreadthFirstSearchDriver<GroundStateModel>::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}

template <>
CSPLiftedStateModel
BreadthFirstSearchDriver<CSPLiftedStateModel>::setup(Problem& problem) const {
	return GroundingSetup::csp_lifted_model(problem);
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
	using EngineT = lapkt::StlBreadthFirstSearch<NodeT, StateModelT, SearchStats>;

    auto model = setup(problem);
	auto engine = std::make_unique<EngineT>(model, _stats, config.getOption<bool>("verbose_stats", false));
	return Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, _stats);
}

// explicit instantiations
template class BreadthFirstSearchDriver<GroundStateModel>;
template class BreadthFirstSearchDriver<CSPLiftedStateModel>;
template class BreadthFirstSearchDriver<SDDLiftedStateModel>;

} // namespaces
