
#pragma once

#include <aptk2/search/interfaces/search_algorithm.hxx>

#include <lifted_state_model.hxx>
#include <actions/action_id.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <search/drivers/registry.hxx>
#include <search/stats.hxx>
#include <utils/config.hxx>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

//! A rather more specific engine creator that simply creates a GBFS planner for lifted planning
class FullyLiftedDriver : public Driver {
protected:
	typedef HeuristicSearchNode<State, LiftedActionID> SearchNode;
	
	SearchStats _stats;
	
public:
	std::unique_ptr<FSLiftedSearchAlgorithm> create(const Config& config, LiftedStateModel& model) const;
	
	LiftedStateModel setup(Problem& problem) const;
	
	void search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

};

} } // namespaces
