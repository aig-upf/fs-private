
#pragma once

#include <search/drivers/registry.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <utils/config.hxx>

namespace fs0 { class GroundStateModel; class Problem; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! An engine creator for the Greedy Best-First Search drivers coupled with our constrained RPG-based heuristics (constrained h_FF, constrained h_max)
//! The choice of the heuristic is done through template instantiation
class NativeDriver : public Driver {
protected:
	typedef HeuristicSearchNode<State, GroundAction> SearchNode;
	
public:

	std::unique_ptr<FSGroundSearchAlgorithm> create(const Config& config, const GroundStateModel& problem) const;
	
	GroundStateModel setup(Problem& problem) const;
	
	void search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

	static bool check_supported(const Problem& problem);
};

} } // namespaces
