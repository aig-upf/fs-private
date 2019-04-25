
#include <fs/core/search/drivers/breadth_first_search.hxx>
#include <fs/core/state.hxx>
#include <fs/core/search/nodes/blind_node.hxx>
#include <fs/core/search/algorithms/breadth_first_search.hxx>
#include <fs/core/search/events.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <lapkt/novelty/novelty_based_acceptor.hxx>


namespace fs0 { namespace drivers {

template <>
GroundStateModel
BreadthFirstSearchDriver<GroundStateModel>::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}


template <typename StateModelT>
ExitCode
BreadthFirstSearchDriver<StateModelT>::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	//! The Breadth-First Search engine uses a simple blind-search node
	using ActionT = typename StateModelT::ActionType;
	using NodeT = lapkt::BlindSearchNode<State, ActionT>;
	using EngineT = lapkt::blai::StlBreadthFirstSearch<NodeT, StateModelT>;
	using OpenListT = lapkt::SearchableQueue<NodeT>;

	auto model = setup(problem);
//	int maxw = config.getOption<int>("width.max", -1);
	auto stop_on_goal = config.getOption<bool>("until_first_goal", false);
	auto max_exp = config.getOption<int>("max_expansions", -1);

	OpenListT queue = OpenListT();
	EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
	auto engine = std::unique_ptr<EngineT>(new EngineT(model, std::move(queue), max_exp, stop_on_goal));
	lapkt::events::subscribe(*engine, _handlers);
	
	return Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, _stats);
}

// explicit instantiations
template class BreadthFirstSearchDriver<GroundStateModel>;

} } // namespaces
