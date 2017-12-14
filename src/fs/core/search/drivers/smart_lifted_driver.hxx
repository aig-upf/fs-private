
#pragma once

#include <fs/core/models/lifted_state_model.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/search/nodes/heuristic_search_node.hxx>
#include <fs/core/search/drivers/registry.hxx>
#include <lapkt/tools/events.hxx>
#include <lapkt/algorithms/best_first_search.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/heuristics/relaxed_plan/smart_rpg.hxx>

namespace fs0 { class Problem; class SearchStats; }

namespace fs0 { namespace drivers {

//! A rather more specific engine creator that simply creates a GBFS planner for lifted planning
class SmartLiftedDriver : public Driver {
protected:
	using NodeT = HeuristicSearchNode<State, LiftedActionID>;
	using HeuristicT = fs0::gecode::SmartRPG;
	using EngineT = lapkt::StlBestFirstSearch<NodeT, LiftedStateModel>;
	using EnginePT = std::unique_ptr<EngineT>;

public:
	EnginePT create(const Config& config, LiftedStateModel& model, SearchStats& stats);
	
	ExitCode search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) override;

protected:
	std::unique_ptr<fs0::gecode::SmartRPG> _heuristic;
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
};

} } // namespaces
