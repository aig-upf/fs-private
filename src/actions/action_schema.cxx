
#include <sstream>

#include <actions/action_schema.hxx>
#include <problem_info.hxx>
#include <actions/ground_action.hxx>
#include <utils/binding.hxx>
#include <utils/printers/actions.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {

ActionSchema::ActionSchema(const std::string& name, const Signature& signature, const std::vector<std::string>& parameters,
						   const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects)
	: BaseAction(0, precondition, effects), _name(name), _signature(signature), _parameters(parameters)
{
	assert(parameters.size() == signature.size());
}


std::string ActionSchema::fullname() const {
     std::ostringstream stream;
     stream << print::schema_name(*this);
     return stream.str();
}

GroundAction* ActionSchema::bind(const Binding& binding, const ProblemInfo& info) const {
	fs::Formula::cptr precondition = _precondition->bind(binding, info);
	
	std::vector<fs::ActionEffect::cptr> effects;
	for (const fs::ActionEffect::cptr effect:_effects) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new GroundAction(this, binding, precondition, effects);
}

std::ostream& ActionSchema::print(std::ostream& os) const { 
	os <<  print::schema_name(*this);
	return BaseAction::print(os);
}

} // namespaces
