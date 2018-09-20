

#include <fs/core/fstrips/fol.hxx>

#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/printers/actions.hxx>


namespace fs0 { namespace fstrips {

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

std::ostream& operator<<(std::ostream &os, const LogicalElement& o) { return o.print(os, LanguageInfo::instance()); }

template <typename T>
std::ostream& _print_nested(std::ostream& os, const LanguageInfo& info, const std::string& name, const std::vector<const T*>& subelements) {
	os << name << "(";
	for (unsigned i = 0; i < subelements.size(); ++i) {
		subelements[i]->print(os, info);
		if (i < subelements.size() - 1) os << ", ";
	}
	os << ")";
	return os;
}

template <typename T>
std::ostream& _print_nested(std::ostream& os, const LanguageInfo& info, unsigned symbol_id, const std::vector<const T*>& subelements) {
	return _print_nested(os, info, info.get_symbol_name(symbol_id), subelements);
}

std::ostream& LogicalVariable::print(std::ostream& os, const LanguageInfo& info) const {
	os << _name;
	return os;
}

std::ostream& Constant::print(std::ostream& os, const LanguageInfo& info) const {
	return os << info.get_object_name(_value);
}


std::ostream& CompositeTerm::print(std::ostream& os, const LanguageInfo& info) const {
	return _print_nested(os, info, _symbol_id, _children);
}

std::ostream& AtomicFormula::print(std::ostream& os, const LanguageInfo& info) const {
	return _print_nested(os, info, _symbol_id, _children);
}

std::ostream& CompositeFormula::print(std::ostream& os, const LanguageInfo& info) const {
	return _print_nested(os, info, to_string(_connective), _children);
}


AtomicFormula::~AtomicFormula() {
	for (const auto ptr:_children) delete ptr;
}

AtomicFormula::AtomicFormula(const AtomicFormula& other) :
	_symbol_id(other._symbol_id),
	_children(Utils::clone(other._children))  // TODO Not clear that we want this behavior
{}

CompositeFormula::~CompositeFormula() {
	for (const auto ptr:_children) delete ptr;
}

CompositeFormula::CompositeFormula(const CompositeFormula& other) :
	_connective(other._connective),
	_children(Utils::clone(other._children))
{}

QuantifiedFormula::~QuantifiedFormula() {
	delete _subformula;
	for (auto ptr:_variables) delete ptr;
}

QuantifiedFormula::QuantifiedFormula(const QuantifiedFormula& other) :
	_quantifier(other._quantifier),
	_variables(Utils::clone(other._variables)),
	_subformula(other._subformula->clone())
{}

CompositeTerm::~CompositeTerm() {
	for (auto ptr:_children) delete ptr;
}

CompositeTerm::CompositeTerm(const CompositeTerm& other) :
		_symbol_id(other._symbol_id),
		_children(Utils::clone(other._children)) // TODO Not clear that we want this behavior
{}

//! Prints a representation of the object to the given stream.
std::ostream& QuantifiedFormula::print(std::ostream& os, const LanguageInfo& info) const {
	os << to_string(_quantifier) << " ";
	for (const LogicalVariable* var:_variables) {
		os << *var << ": " << info.get_typename(var->getType()) << " ";
	}
	os << " s.t. ";
	os << "(" << *_subformula << ")";
	return os;
}


} } // namespaces
