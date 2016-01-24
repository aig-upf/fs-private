
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <asp/models/base_model.hxx>

namespace fs0 { namespace language { namespace fstrips {
	class AtomicFormula; class ActionEffect;
} } }

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace asp {

class ChoiceRuleModel : public BaseModel {
protected:
	
	//!
	std::vector<const fs::AtomicFormula*> _goal_atoms;
	
	//!
	ActionIdx _action_index;
	
	//!
	void process_ground_action(const GroundAction& action, std::vector<std::string>& rules) const;
	
	std::pair<std::string, bool> process_atom(const fs::AtomicFormula* atom) const;
	std::pair<std::string, bool> process_effect(const fs::ActionEffect* effect) const;

public:
	ChoiceRuleModel(const Problem& problem);
	
	std::vector<std::string> build_domain_rules(bool optimize) const;
	
	std::vector<std::string> build_state_rules(const State& state) const;

	const ActionIdx& get_action_idx() const { return _action_index; }

};

} } // namespaces
