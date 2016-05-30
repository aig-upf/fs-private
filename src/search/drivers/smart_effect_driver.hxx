
#pragma once

#include <search/algorithms/ehc_gbfs.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <utils/config.hxx>

#include <search/algorithms/ehc.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>

namespace fs0 { class GroundStateModel;}

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! An engine creator for the Greedy Best-First Search drivers coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
//! The choice of the heuristic is done through template instantiation
class SmartEffectDriver {
protected:
	typedef HeuristicSearchNode<State, GroundAction> SearchNode;
	
	typedef std::unique_ptr<EHCStateModel<fs0::gecode::SmartRPG>> Model;
	typedef std::unique_ptr<EHCThenGBFSSearch<fs0::gecode::SmartRPG>> Engine;
	
public:
	SmartEffectDriver() {}
	
	std::unique_ptr<EHCThenGBFSSearch<fs0::gecode::SmartRPG>> create(const Config& config, const GroundStateModel& problem);
	
	GroundStateModel setup(const Config& config, Problem& problem) const;
};

} } // namespaces
