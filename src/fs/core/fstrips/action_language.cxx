

#include <fs/core/fstrips/action_language.hxx>

#include <fs/core/fstrips/fol.hxx>
#include <fs/core/fstrips/fol_syntax.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/printers/actions.hxx>


namespace fs0::fstrips {


ActionEffect::~ActionEffect() { delete _condition; }

ActionEffect::ActionEffect(const ActionEffect& other) :
	_condition(other._condition->clone())
{}

FunctionalEffect::~FunctionalEffect() { delete _lhs; delete _rhs; }

FunctionalEffect::FunctionalEffect(const FunctionalEffect& other) :
	ActionEffect(other), _lhs(other._lhs->clone()), _rhs(other._rhs->clone())
{}

const Term* FunctionalEffect::lhs() const { return _lhs; }
const Term* FunctionalEffect::rhs() const { return _rhs; }

AtomicEffect::~AtomicEffect() { delete _atom; }

AtomicEffect::AtomicEffect(const AtomicEffect& other) :
	ActionEffect(other), _atom(other._atom->clone()), _type(other._type)
{}

AtomicEffectType AtomicEffect::to_type(const std::string& type) {
	if (type == "add") return AtomicEffectType::ADD;
	if (type == "del") return AtomicEffectType::DEL;
	throw std::runtime_error("Unknown effect type" + type);	
}

std::ostream& FunctionalEffect::print(std::ostream& os, const LanguageInfo& info) const {
    if (_condition) {
        _condition->print(os, info);
        os << " -> ";
    }
    _lhs->print(os, info);
    os << " := ";
    _rhs->print(os, info);
	return os;
}

std::ostream& AtomicEffect::print(std::ostream& os, const LanguageInfo& info) const {
	if (_condition) {
	    _condition->print(os, info);
		os << " -> ";
	}
	if (_type == AtomicEffectType::DEL) os << "~";
    _atom->print(os, info);
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


} // namespaces
