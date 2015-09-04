
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>

namespace fs0 { class FS0StateModel; class Config; }

namespace fs0 { namespace engines {

//! A creator for the Greedy Best-First Search FS0 engine
class GBFSEngineCreator : public EngineCreator {
protected:
	typedef HeuristicSearchNode<fs0::State> SearchNode;
	
public:
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const;
	
protected:
	//! Check whether a Gecode RPG builder is imperative
	static bool needsGecodeRPGBuilder(const std::vector<fs::AtomicFormula::cptr>& goal_conditions, const std::vector<fs::AtomicFormula::cptr>& state_constraints);
};

} } // namespaces
