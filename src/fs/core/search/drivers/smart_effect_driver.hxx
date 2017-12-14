
#pragma once

#include <fs/core/search/algorithms/ehc_gbfs.hxx>
#include <fs/core/search/nodes/heuristic_search_node.hxx>
#include <fs/core/utils/config.hxx>

#include <fs/core/search/algorithms/ehc.hxx>
#include <fs/core/search/events.hxx>
#include <fs/core/heuristics/relaxed_plan/smart_rpg.hxx>
#include <lapkt/algorithms/best_first_search.hxx>

namespace fs0 { class GroundStateModel; class SearchStats; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! An engine creator for the Greedy Best-First Search drivers coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
//! The choice of the heuristic is done through template instantiation
class SmartEffectDriver : public Driver {
public:
	using NodeT = HeuristicSearchNode<State, GroundAction>;
	using GBFST = lapkt::StlBestFirstSearch<NodeT, GroundStateModel>;
	using EngineT = EHCThenGBFSSearch<GBFST, gecode::SmartRPG>;
	using EnginePT = std::unique_ptr<EngineT>;
	
protected:
	std::unique_ptr<gecode::SmartRPG> _heuristic;
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
public:
	SmartEffectDriver() {}
	
	EnginePT create(const Config& config, const GroundStateModel& problem, SearchStats& stats);
	
	static gecode::SmartRPG* configure_heuristic(const Problem& problem, const Config& config);
	
	static GroundStateModel setup(Problem& problem);
	
	ExitCode search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) override;
};

} } // namespaces
