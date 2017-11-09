
#pragma once

#include <fs/core/models/lifted_state_model.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/search/nodes/heuristic_search_node.hxx>
#include <fs/core/search/drivers/registry.hxx>
#include <lapkt/tools/events.hxx>
#include <lapkt/algorithms/best_first_search.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/heuristics/relaxed_plan/gecode_crpg.hxx>

namespace fs0 { class Problem; class SearchStats; }

namespace fs0 { namespace drivers {

//! A rather more specific engine creator that simply creates a GBFS planner for lifted planning
class FullyLiftedDriver : public Driver {
public:
	using NodeT = HeuristicSearchNode<State, LiftedActionID>;
	using HeuristicT = fs0::gecode::GecodeCRPG;
	using EngineT = lapkt::StlBestFirstSearch<NodeT, LiftedStateModel>;
	using EnginePT = std::unique_ptr<EngineT>;
	
	
	EnginePT create(const Config& config, LiftedStateModel& model, SearchStats& stats);
	
	LiftedStateModel setup(Problem& problem) const;
	
	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

protected:
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	std::unique_ptr<HeuristicT> _heuristic;
};

} } // namespaces
