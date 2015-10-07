
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <utils/config.hxx>

namespace fs0 { class FS0StateModel; }

namespace fs0 { namespace engines {

//! An engine creator for the Greedy Best-First Search engines coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
//! The choice of the heuristic is done thrugh template instantiation
template <typename GecodeHeuristic, typename DirectHeuristic>
class GBFSConstrainedHeuristicsCreator : public EngineCreator {
protected:
	typedef HeuristicSearchNode<fs0::State> SearchNode;
	
public:
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const;
	
protected:
	
	static Config::CSPManagerType decide_csp_type(const Problem& problem);
	
	static Config::CSPManagerType decide_action_manager_type(const std::vector< fs0::GroundAction::cptr >& actions);
	
	static Config::CSPManagerType decide_builder_type(const std::vector<fs::AtomicFormula::cptr>& goal_conditions, const std::vector<fs::AtomicFormula::cptr>& state_constraints);

};

} } // namespaces
