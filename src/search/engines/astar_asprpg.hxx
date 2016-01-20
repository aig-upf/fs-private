
#pragma once

#include <search/engines/registry.hxx>
#include <search/nodes/astar_search_node.hxx>
#include <utils/config.hxx>

namespace fs0 { class FS0StateModel;}

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace engines {

//! An engine creator for an AStar search coupled with the (optimal) ASP-based h^+ admissible heuristic.
class AStarASPRPG : public EngineCreator {
public:
	std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& problem) const;
	
protected:
	//! Check that the problem is suitable for this type of engine
	static void validate(const Problem& problem);
};

} } // namespaces
