

#include "light.hxx"

#include <problem_info.hxx>
#include <utils/utils.hxx>
#include <utils/printers/actions.hxx>


namespace fs0 { namespace lang { namespace fstrips {

//! Some printing helpers
const std::string to_string(Connective connective) {
	if (connective == Connective::Conjunction) return "and";
	if (connective == Connective::Disjunction) return "or";
	if (connective == Connective::Negation) return "not";
	throw std::runtime_error("Unknown connective");
}

const std::string to_string(Quantifier quantifier) {
	if (quantifier == Quantifier::Universal) return "forall";
	if (quantifier == Quantifier::Existential) return "exists";
	throw std::runtime_error("Unknown quantifier");
}

const Connective to_connective(const std::string& connective) {
	if (connective == "and") return Connective::Conjunction;
	if (connective == "or") return Connective::Disjunction;
	if (connective == "not") return Connective::Negation;
	throw std::runtime_error("Unknown connective");
}

const Quantifier to_quantifier(const std::string& quantifier) {
	if (quantifier == "forall") return Quantifier::Universal;
	if (quantifier == "exists") return Quantifier::Existential;
	throw std::runtime_error("Unknown quantifier");	
}

template <typename T>
std::ostream& _print_nested(std::ostream& os, const std::string& name, const std::vector<const T*>& subelements) {
	os << name << "(";
	for (unsigned i = 0; i < subelements.size(); ++i) {
		os << *subelements[i];
		if (i < subelements.size() - 1) os << ", ";
	}
	os << ")";
	return os;
}

template <typename T>
std::ostream& _print_nested(std::ostream& os, const fs0::ProblemInfo& info, unsigned symbol_id, const std::vector<const T*>& subelements) {
	return _print_nested(os, info.getSymbolName(symbol_id), subelements);
}

std::ostream& LogicalVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "?" << _id;
	return os;
}

std::ostream& Constant::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.deduceObjectName(_value, _type);
	return os;
}


std::ostream& FunctionalTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _print_nested(os, info, _symbol_id, _children);
}

std::ostream& AtomicFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _print_nested(os, info, _symbol_id, _children);
}

std::ostream& OpenFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _print_nested(os, to_string(_connective), _children);
}


FunctionalTerm::FunctionalTerm(const FunctionalTerm& other) :
	_symbol_id(other._symbol_id),
	_children(Utils::clone(other._children))
{}

AtomicFormula::AtomicFormula(const AtomicFormula& other) :
	_symbol_id(other._symbol_id),
	_children(Utils::clone(other._children))
{}

OpenFormula::OpenFormula(const OpenFormula& other) :
	_connective(other._connective),
	_children(Utils::clone(other._children))
{}

QuantifiedFormula::QuantifiedFormula(const QuantifiedFormula& other) :
	_quantifier(other._quantifier),
	_variables(Utils::clone(other._variables)),
	_subformula(other._subformula->clone())
{}

//! Prints a representation of the object to the given stream.
std::ostream& QuantifiedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << to_string(_quantifier) << " ";
	for (const LogicalVariable* var:_variables) {
		os << *var << ": " << info.getTypename(var->getType()) << " s.t. ";
	}
	os << "(" << *_subformula << ")";
	return os;
}

///////////////////////////////////////////////////////////////////////////////


ActionEffect::ActionEffect(const ActionEffect& other) :
	_condition(other._condition->clone())
{}

FunctionalEffect::FunctionalEffect(const FunctionalEffect& other) :
	ActionEffect(other), _lhs(other._lhs->clone()), _rhs(other._rhs->clone())
{}

AtomicEffect::AtomicEffect(const AtomicEffect& other) :
	ActionEffect(other), _atom(other._atom->clone()), _type(other._type)
{}

std::ostream& ActionEffect::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

std::ostream& FunctionalEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	if (_condition) {
		os << *_condition << " --> ";
	}
	os << *_lhs << " := " << *_rhs;
	return os;
}

std::ostream& AtomicEffect::print(std::ostream& os, const fs0::ProblemInfo& info) const {
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

std::ostream& ActionSchema::print(std::ostream& os) const { 
	return os <<  print::action_signature(*this);
}


} } } // namespaces
