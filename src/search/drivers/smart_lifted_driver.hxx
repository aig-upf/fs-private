
#pragma once

#include <aptk2/search/interfaces/search_algorithm.hxx>

#include <lifted_state_model.hxx>
#include <actions/action_id.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <search/drivers/registry.hxx>
#include <utils/config.hxx>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

//! A rather more specific engine creator that simply creates a GBFS planner for lifted planning
class SmartLiftedDriver {
protected:
	typedef HeuristicSearchNode<State, LiftedActionID> SearchNode;
	
public:
	std::unique_ptr<FSLiftedSearchAlgorithm> create(const Config& config, LiftedStateModel& model) const;
	
	LiftedStateModel setup(const Config& config, Problem& problem) const;
};

} } // namespaces
