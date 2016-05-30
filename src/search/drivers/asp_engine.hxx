
#pragma once

#include <search/drivers/registry.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! An engine creator for an AStar search coupled with the (optimal) ASP-based h^+ admissible heuristic.
class ASPEngine : public Driver {
public:
	std::unique_ptr<FSGroundSearchAlgorithm> create(const Config& config, const GroundStateModel& problem) const;
	
protected:
	//! Check that the problem is suitable for this type of engine
	static void validate(const Problem& problem);
};

} } // namespaces
