
#pragma once

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

//! A rather more specific engine creator that simply creates a GBFS planner for lifted planning
class Validation {
public:
	static void check_no_conditional_effects(const Problem& problem);
};

} } // namespaces
