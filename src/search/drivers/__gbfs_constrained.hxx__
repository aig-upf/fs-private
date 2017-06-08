
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <lapkt/tools/events.hxx>
#include <utils/config.hxx>

#include <state.hxx>
#include <actions/actions.hxx>

namespace fs0 { namespace gecode { class GecodeCRPG; } }


// namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
// namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! An engine creator for the Greedy Best-First Search drivers coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
//! The choice of the heuristic is done through template instantiation
class GBFS_CRPGDriver : public Driver {
public:
	using NodeT = HeuristicSearchNode<State, GroundAction>;

	~GBFS_CRPGDriver();

	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
	
protected:
	//!
	gecode::GecodeCRPG* _heuristic;

	//!
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
};

} } // namespaces
