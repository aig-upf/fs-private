
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <lapkt/algorithms/best_first_search.hxx>
#include <utils/config.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>
#include <lifted_state_model.hxx>

namespace fs0 { class GroundStateModel; class SearchStats; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

template <typename StateModelT>
class UnreachedAtomDriver : public Driver {
public:
	using ActionT = typename StateModelT::ActionType;
	using NodeT = HeuristicSearchNode<State, ActionT>;
	using HeuristicT = fs0::gecode::UnreachedAtomRPG;
	using EngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicT, StateModelT>;
	using EnginePT = std::unique_ptr<EngineT>;
	
	EnginePT create(const Config& config, const StateModelT& problem, SearchStats& stats);
	
	StateModelT setup(Problem& problem) const;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
protected:
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	std::unique_ptr<HeuristicT> _heuristic;
};

// explicit instantiations
// template class UnreachedAtomDriver<GroundStateModel>;
template class UnreachedAtomDriver<LiftedStateModel>;


} } // namespaces
