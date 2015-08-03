
#include <actions/action_schema.hxx>
#include <problem.hxx>
#include <actions/ground_action.hxx>
#include <utils/printers/binding.hxx>

namespace fs0 {

ActionSchema::ActionSchema(const std::string& name, const std::string& classname,
						   const std::vector<TypeIdx>& signature, const std::vector<std::string>& parameters,
						   const std::vector<AtomicFormulaSchema::cptr>& conditions, const std::vector<ActionEffectSchema::cptr>& effects)
	: _name(name), _classname(classname), _signature(signature), _parameters(parameters), _conditions(conditions), _effects(effects)
{
	assert(parameters.size() == signature.size());
}


ActionSchema::~ActionSchema() {
	for (const auto ptr:_conditions) delete ptr;
	for (const auto ptr:_effects) delete ptr;
}

std::ostream& ActionSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os <<  _name << "(" << print::signature(_parameters, getSignature()) << ")" << std::endl;
	
	os << "Preconditions:" << std::endl;
	for (auto elem:_conditions) os << "\t" << *elem << std::endl;
	
	os << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t" << *elem << std::endl;
	return os;
}

GroundAction* ActionSchema::process(const ObjectIdxVector& binding, const ProblemInfo& info) const {
	std::vector<AtomicFormula::cptr> conditions;
	for (const AtomicFormulaSchema::cptr condition:_conditions) {
		AtomicFormula::cptr processed = condition->process(binding, info);
		
		// Static checks
		if (processed->is_tautology()) { // No need to add the condition, which is always true
			delete processed;
			continue; 
		} else if (processed->is_contradiction()) { // The action is statically non-applicable
			delete processed;
			for (const auto c:conditions) delete c;
			return nullptr;
		}
		
		conditions.push_back(processed);
	}
	
	std::vector<ActionEffect::cptr> effects;
	for (const ActionEffectSchema::cptr effect:_effects) {
		effects.push_back(effect->process(binding, info));
	}
	
	return new GroundAction(this, binding, conditions, effects);
}

} // namespaces
