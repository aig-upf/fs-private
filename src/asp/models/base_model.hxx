
#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace fs0 {
	class Problem; class GroundAction; class State;
}

namespace fs0 { namespace asp {

class BaseModel {
protected:
	//! The problem instance
	const Problem& _problem;
	
public:
	typedef std::unordered_map<std::string, unsigned> ActionIdx;
	
	BaseModel(const Problem& problem) : _problem(problem) {}
	virtual ~BaseModel() = 0;
	
	//! Build the LP rules that correspond to the problem domain & goal
	virtual std::vector<std::string> build_domain_rules(bool optimize) const = 0;
	
	//! Build the LP rules that correspond to the given state
	virtual std::vector<std::string> build_state_rules(const State& state) const = 0;
	
	//! Returns the set of ground action IDs which are part of the relaxed plan
	//! represented by a given ASP solution
	virtual const ActionIdx& get_action_idx() const = 0;
};

} } // namespaces
