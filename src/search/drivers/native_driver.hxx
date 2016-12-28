
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <lapkt/events.hxx>
#include <lapkt/algorithms/best_first_search.hxx>
#include <utils/config.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <ground_state_model.hxx>

namespace fs0 { class GroundStateModel; class Problem; class SearchStats; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! An engine creator for the Greedy Best-First Search drivers coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
template <typename StateModelT>
class NativeDriver : public Driver {
public:
	using ActionT = typename StateModelT::ActionType;
	using NodeT = HeuristicSearchNode<State, ActionT>;
	using HeuristicT = DirectCRPG;
	using EngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicT, StateModelT>;
	using EnginePT = std::unique_ptr<EngineT>;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

	EnginePT create(const Config& config, const StateModelT& problem, SearchStats& stats);
	
	StateModelT setup(Problem& problem) const;
	
protected:
	static bool check_supported(const Problem& problem);
	
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	std::unique_ptr<HeuristicT> _heuristic;
};

// explicit instantiations
template class NativeDriver<GroundStateModel>;
// template class NativeDriver<LiftedStateModel>;

} } // namespaces
