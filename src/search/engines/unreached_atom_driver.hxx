
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <utils/config.hxx>

namespace fs0 { class FS0StateModel;}

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace engines {

//! An engine creator for the Greedy Best-First Search engines coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
//! The choice of the heuristic is done through template instantiation
class UnreachedAtomDriver : public EngineCreator {
protected:
	typedef HeuristicSearchNode<State, GroundAction> SearchNode;
	
public:
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& problem) const;
	
	void setup(const Config& config, Problem& problem) const;
	
};

} } // namespaces
