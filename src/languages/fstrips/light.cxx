

#include "light.hxx"

#include <problem_info.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {

//! Some printing helpers
const std::string to_string(Connective connective) {
	if (connective == Connective::Conjunction) return "and";
	if (connective == Connective::Disjunction) return "or";
	if (connective == Connective::Negation) return "not";
	throw std::runtime_error("Unknown connective");
}

const std::string to_string(Quantifier connective) {
	if (connective == Quantifier::Universal) return "forall";
	if (connective == Quantifier::Existential) return "exists";
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

std::ostream& LConstant::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.deduceObjectName(_value, _type);
	return os;
}


std::ostream& FunctionalTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _print_nested(os, info, _symbol_id, _children);
}

std::ostream& LAtomicFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _print_nested(os, info, _symbol_id, _children);
}

std::ostream& OpenLFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return _print_nested(os, to_string(_connective), _children);
}


FunctionalTerm::FunctionalTerm(const FunctionalTerm& other) :
	_symbol_id(other._symbol_id),
	_children(Utils::clone(other._children))
{}

LAtomicFormula::LAtomicFormula(const LAtomicFormula& other) :
	_symbol_id(other._symbol_id),
	_children(Utils::clone(other._children))
{}

OpenLFormula::OpenLFormula(const OpenLFormula& other) :
	_connective(other._connective),
	_children(Utils::clone(other._children))
{}

LQuantifiedFormula::LQuantifiedFormula(const LQuantifiedFormula& other) :
	_quantifier(other._quantifier),
	_variables(Utils::clone(other._variables)),
	_subformula(other._subformula->clone())
{}

//! Prints a representation of the object to the given stream.
std::ostream& LQuantifiedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << to_string(_quantifier) << " ";
	for (const LogicalVariable* var:_variables) {
		os << *var << ": " << info.getTypename(var->getType()) << " s.t. ";
	}
	os << "(" << *_subformula << ")";
	return os;
}

} } } // namespaces
