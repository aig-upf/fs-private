

#include <fs/core/fstrips/language.hxx>

#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/printers/actions.hxx>


namespace fs0 { namespace fstrips {


ActionEffect::ActionEffect(const ActionEffect& other) :
	_condition(other._condition->clone())
{}

FunctionalEffect::FunctionalEffect(const FunctionalEffect& other) :
	ActionEffect(other), _lhs(other._lhs->clone()), _rhs(other._rhs->clone())
{}

AtomicEffect::AtomicEffect(const AtomicEffect& other) :
	ActionEffect(other), _atom(other._atom->clone()), _type(other._type)
{}

AtomicEffect::Type AtomicEffect::to_type(const std::string& type) {
	if (type == "add") return Type::ADD;
	if (type == "del") return Type::DEL;
	throw std::runtime_error("Unknown effect type" + type);	
}

std::ostream& FunctionalEffect::print(std::ostream& os, const LanguageInfo& info) const {
	if (_condition) {
		os << *_condition << " --> ";
	}
	os << *_lhs << " := " << *_rhs;
	return os;
}

std::ostream& AtomicEffect::print(std::ostream& os, const LanguageInfo& info) const {
	if (_condition) {
		os << *_condition << " --> ";
	}
	if (_type == Type::DEL) os << "~";
	os << *_atom;
	return os;
}


ActionSchema::ActionSchema(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameter_names,
                           const Formula* precondition, const std::vector<const ActionEffect*>& effects)
	: _id(id), _name(name), _signature(signature), _parameter_names(parameter_names), _precondition(precondition), _effects(effects)
{
	assert(parameter_names.size() == signature.size());
}

ActionSchema::~ActionSchema() {
	delete _precondition;
	for (const auto pointer:_effects) delete pointer;
}

ActionSchema::ActionSchema(const ActionSchema& other) :
	_id(other._id),
	_name(other._name),
	_signature(other._signature),
	_parameter_names(other._parameter_names),
	_precondition(other._precondition->clone()),
	_effects(Utils::clone(other._effects))
{}

std::ostream& operator<<(std::ostream &os, const ActionEffect& o)   { return o.print(os, LanguageInfo::instance()); }

std::ostream& ActionSchema::print(std::ostream& os) const { 
	return os <<  print::action_signature(*this);
}


} } // namespaces
