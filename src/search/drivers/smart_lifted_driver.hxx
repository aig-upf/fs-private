
#pragma once

#include <lifted_state_model.hxx>
#include <actions/action_id.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <search/drivers/registry.hxx>
#include <lapkt/events.hxx>
#include <lapkt/algorithms/best_first_search.hxx>
#include <utils/config.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>

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
	
	LiftedStateModel setup(Problem& problem) const;

	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

protected:
	std::unique_ptr<fs0::gecode::SmartRPG> _heuristic;
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
};

} } // namespaces
