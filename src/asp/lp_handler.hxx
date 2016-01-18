
#pragma once

#include <vector>
#include <string>

namespace fs0 { class Problem; class State; }

namespace fs0 { namespace asp {

class LPHandler {
public:
	LPHandler() {}
	
	//! Add logic rules corresponding to the problem domain
	virtual void on_domain_rules(const Problem& problem, std::vector<std::string>& rules) const = 0;

	//! Add logic rules corresponding to the given state
	virtual void on_state_rules(const Problem& problem, const State& seed, std::vector<std::string>& rules) const = 0;
};

} } // namespaces
