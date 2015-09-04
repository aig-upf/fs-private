
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
	
	//! Check whether a Gecode RPG builder is imperative
	static bool needsGecodeRPGBuilder(const std::vector<fs::AtomicFormula::cptr>& goal_conditions, const std::vector<fs::AtomicFormula::cptr>& state_constraints);
	
	static Config::GoalManagerType decideRPGBuilderType(const Problem& problem);
};

} } // namespaces
