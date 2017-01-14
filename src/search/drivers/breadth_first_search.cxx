
#include <search/drivers/breadth_first_search.hxx>
#include <state.hxx>
#include <lapkt/nodes/blind_node.hxx>
#include <lapkt/algorithms/breadth_first_search.hxx>
#include <search/events.hxx>
#include <search/utils.hxx>
#include <search/drivers/setups.hxx>


namespace fs0 { namespace drivers {

template <>
GroundStateModel
BreadthFirstSearchDriver<GroundStateModel>::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}

template <>
LiftedStateModel
BreadthFirstSearchDriver<LiftedStateModel>::setup(Problem& problem) const {
	return GroundingSetup::fully_lifted_model(problem);
}

template <typename StateModelT>
ExitCode
BreadthFirstSearchDriver<StateModelT>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	//! The Breadth-First Search engine uses a simple blind-search node
	using ActionT = typename StateModelT::ActionType;
	using NodeT = lapkt::BlindSearchNode<State, ActionT>;
	using EngineT = lapkt::StlBreadthFirstSearch<NodeT, StateModelT>;
	using EnginePT = std::unique_ptr<EngineT>;

	auto model = setup(problem);
	
	EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
	auto engine = EnginePT(new EngineT(model));
	lapkt::events::subscribe(*engine, _handlers);
	
	return Utils::do_search(*engine, model, out_dir, start_time, _stats);
}

// explicit instantiations
template class BreadthFirstSearchDriver<GroundStateModel>;
template class BreadthFirstSearchDriver<LiftedStateModel>;

} } // namespaces
