
#include <sstream>

#include <actions/action_schema.hxx>
#include <problem.hxx>
#include <actions/ground_action.hxx>
#include <utils/binding.hxx>
#include <utils/printers/actions.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {

ActionSchema::ActionSchema(const std::string& name,
						   const Signature& signature, const std::vector<std::string>& parameters,
						   const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects)
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
	os <<  print::schema_name(*this);
	
	os << "Precondition:" << *_precondition << std::endl;
	
	os << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t" << *elem << std::endl;
	return os;
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

} // namespaces
