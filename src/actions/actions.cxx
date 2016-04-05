
#include <limits>
#include <sstream>

#include <actions/actions.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <utils/binding.hxx>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <languages/fstrips/language.hxx>


namespace fs0 {

BaseAction::BaseAction(unsigned id, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects)
	: _id(id), _precondition(precondition), _effects(effects)
{}


BaseAction::~BaseAction() {
	delete _precondition;
	for (const auto pointer:_effects) delete pointer;
}

std::ostream& BaseAction::print(std::ostream& os) const {
	os << "\t" << "Precondition:" << *_precondition <<  std::endl;
	os << "\t" << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t\t" << *elem << std::endl;
	return os;
}


ActionSchema::ActionSchema(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameters,
						   const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects)
	: BaseAction(id, precondition, effects), _name(name), _signature(signature), _parameters(parameters)
{
	assert(parameters.size() == signature.size());
}


std::string ActionSchema::fullname() const {
     std::ostringstream stream;
     stream << print::schema_name(*this);
     return stream.str();
}

GroundAction* ActionSchema::bind(unsigned id, const Binding& binding, const ProblemInfo& info) const {
	fs::Formula::cptr precondition = _precondition->bind(binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}
	
	std::vector<fs::ActionEffect::cptr> effects;
	for (const fs::ActionEffect::cptr effect:_effects) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new GroundAction(id, this, binding, precondition, effects);
}

GroundAction* ActionSchema::bind(const Binding& binding, const ProblemInfo& info) const { return bind(GroundAction::invalid_action_id, binding, info); }

ActionSchema* ActionSchema::process(const ProblemInfo& info) const {
	Binding binding; // An empty binding
	auto precondition = _precondition->bind(binding, info);
	if (precondition->is_contradiction()) {
		throw std::runtime_error("The precondition of the action schema is (statically) unsatisfiable!");
	}
	
	std::vector<fs::ActionEffect::cptr> effects;
	for (const fs::ActionEffect::cptr effect:_effects) {
		effects.push_back(effect->bind(binding, info));
	}
	return new ActionSchema(_id, _name, _signature, _parameters, precondition, effects);
}


std::ostream& ActionSchema::print(std::ostream& os) const { 
	os <<  print::schema_name(*this);
	return BaseAction::print(os);
}

const ActionIdx GroundAction::invalid_action_id = std::numeric_limits<unsigned int>::max();

GroundAction::GroundAction(unsigned id, ActionSchema::cptr schema, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects)
	: BaseAction(id, precondition, effects), _schema(schema), _binding(binding)
{}


const std::string& GroundAction::getName() const { return _schema->getName(); }

const Signature& GroundAction::getSignature() const { return _schema->getSignature(); }

std::string GroundAction::fullname() const {
     std::ostringstream stream;
     stream << print::action_name(*this);
     return stream.str();
}

bool GroundAction::isBound(unsigned i) const { return _binding.binds(i); }

std::ostream& GroundAction::print(std::ostream& os) const {
	os << print::action_name(*this) << std::endl;
	return BaseAction::print(os);
}

} // namespaces
