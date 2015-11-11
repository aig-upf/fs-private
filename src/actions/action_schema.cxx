
#include <actions/action_schema.hxx>
#include <problem.hxx>
#include <actions/ground_action.hxx>
#include <utils/printers/binding.hxx>

namespace fs0 {

ActionSchema::ActionSchema(const std::string& name,
						   const Signature& signature, const std::vector<std::string>& parameters,
						   const Formula::cptr precondition, const std::vector<ActionEffect::cptr>& effects)
	: _name(name), _signature(signature), _parameters(parameters), _precondition(precondition), _effects(effects)
{
	assert(parameters.size() == signature.size());
}


ActionSchema::~ActionSchema() {
	delete _precondition;
	for (const auto ptr:_effects) delete ptr;
}

std::ostream& ActionSchema::print(std::ostream& os) const { return print(os, Problem::getInfo()); }

std::ostream& ActionSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os <<  _name << "(" << print::signature(_parameters, getSignature()) << ")" << std::endl;
	
	os << "Precondition:" << *_precondition << std::endl;
	
	os << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t" << *elem << std::endl;
	return os;
}

GroundAction* ActionSchema::bind(const Binding& binding, const ProblemInfo& info) const {
	Formula::cptr precondition = _precondition->bind(binding, info);
	
	std::vector<ActionEffect::cptr> effects;
	for (const ActionEffect::cptr effect:_effects) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new GroundAction(this, binding, precondition, effects);
}

} // namespaces
