
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <utils/config.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>

namespace fs0 { class GroundStateModel; class SearchStats; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

class UnreachedAtomDriver : public Driver {
public:
	using NodeT = HeuristicSearchNode<State, GroundAction>;
	using HeuristicT = fs0::gecode::UnreachedAtomRPG;
	using EngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicT, GroundStateModel>;
	using EnginePT = std::unique_ptr<EngineT>;
	
	EnginePT create(const Config& config, const GroundStateModel& problem, SearchStats& stats);
	
	GroundStateModel setup(Problem& problem) const;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
protected:
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	std::unique_ptr<HeuristicT> _heuristic;
};

} } // namespaces
