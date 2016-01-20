
#pragma once

#include <vector>
#include <string>
#include <unordered_map>


namespace Gringo { struct Value; }

namespace fs0 {
	class Problem; class GroundAction; class State;
}

namespace fs0 { namespace language { namespace fstrips {
	class AtomicFormula; class ActionEffect;
} } }

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace asp {

class LPHandler;

class Model {
protected:
	
	bool _optimize;
	
	//!
	std::vector<const fs::AtomicFormula*> _goal_atoms;
	
	//!
	std::unordered_map<std::string, unsigned> _action_index;
	
	//!
	const Problem& _problem;
	
	//!
	void process_ground_action(const GroundAction& action, std::vector<std::string>& rules) const;
	
	std::pair<std::string, bool> process_atom(const fs::AtomicFormula* atom) const;
	std::pair<std::string, bool> process_effect(const fs::ActionEffect* effect) const;



public:
	Model(const Problem& problem);
	
	std::vector<std::string> build_domain_rules(bool optimize) const;
	
	std::vector<std::string> build_state_rules(const State& state) const;
	
	//! Returns the set of ground action IDs which are part of the relaxed plan
	//! represented by a given ASP solution
	std::vector<unsigned> get_action_set(const std::vector<Gringo::Value>& model) const;

};

} } // namespaces
