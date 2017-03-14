
#include <boost/functional/hash.hpp>

#include <problem_info.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <lapkt/tools/logging.hxx>
#include <utils/binding.hxx>

#include <languages/fstrips/operations/interpretation.hxx>

namespace fs0 { namespace language { namespace fstrips {

ObjectIdx Term::interpret(const PartialAssignment& assignment) const { return interpret(assignment, Binding::EMPTY_BINDING); }
ObjectIdx Term::interpret(const State& state) const  { return interpret(state, Binding::EMPTY_BINDING); }



NestedTerm::NestedTerm(const NestedTerm& term) :
	_symbol_id(term._symbol_id),
	_subterms(Utils::clone(term._subterms)),
	_interpreted_subterms(term._interpreted_subterms)
{}


StaticHeadedNestedTerm::StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<const Term*>& subterms)
	: NestedTerm(symbol_id, subterms)
{}

UserDefinedStaticTerm::UserDefinedStaticTerm(unsigned symbol_id, const std::vector<const Term*>& subterms)
	: StaticHeadedNestedTerm(symbol_id, subterms),
	_function(ProblemInfo::getInstance().getSymbolData(symbol_id))
{}


ArithmeticTerm::ArithmeticTerm(const std::vector<const Term*>& subterms)
	: StaticHeadedNestedTerm(-1, subterms)
{
	assert(subterms.size() == 2);
}



AxiomaticTerm* AxiomaticTerm::clone() const { return clone(Utils::clone(_subterms)); }



ObjectIdx UserDefinedStaticTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	interpret_subterms(_subterms, assignment, binding, _interpreted_subterms);
	return _function.getFunction()(_interpreted_subterms);
}

ObjectIdx UserDefinedStaticTerm::interpret(const State& state, const Binding& binding) const {
	interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return _function.getFunction()(_interpreted_subterms);
}


ObjectIdx AxiomaticTerm::interpret(const State& state, const Binding& binding) const {
	interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return compute(state, _interpreted_subterms);
}


ObjectIdx FluentHeadedNestedTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	return assignment.at(fs::interpret_variable(*this, assignment, binding));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const State& state, const Binding& binding) const {
	return state.getValue(fs::interpret_variable(*this, state, binding));
}


ObjectIdx StateVariable::interpret(const State& state, const Binding& binding) const {
	return state.getValue(_variable_id);
}


ObjectIdx BoundVariable::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	if (!binding.binds(_id)) throw std::runtime_error("Cannot interpret bound variable without a suitable binding");
	return binding.value(_id);
}

ObjectIdx BoundVariable::interpret(const State& state, const Binding& binding) const {
	if (!binding.binds(_id)) throw std::runtime_error("Cannot interpret bound variable without a suitable binding");
	return binding.value(_id);
}



//! A quick helper to print functions
template <typename T>
std::ostream& printFunction(std::ostream& os, const fs0::ProblemInfo& info, unsigned symbol_id, const std::vector<T*>& subterms) {
	os << info.getSymbolName(symbol_id) << "(";
	for (unsigned i = 0; i < subterms.size(); ++i) {
		os << *subterms[i];
		if (i < subterms.size() - 1) os << ", ";
	}
	os << ")";
	return os;
}

std::ostream& Term::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "<unnamed term>";
	return os;
}

std::ostream& NestedTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return printFunction(os, info, _symbol_id, _subterms);
}

std::ostream& StateVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.getVariableName(_variable_id);
	return os;
}

std::ostream& BoundVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "?" << _id;
	return os;
}
std::ostream& Constant::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.getCustomObjectName(_value); // We are sure that this is a custom object, otherwise the IntConstant::print() would be executed
	return os;
}

std::ostream& IntConstant::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << _value;
	return os;
}

bool NestedTerm::operator==(const Term& other) const {
	auto derived = dynamic_cast<const NestedTerm*>(&other);
	if (!derived
		|| _symbol_id != derived->_symbol_id
		|| _subterms.size() != derived->_subterms.size()) {
		return false;
	}

	for (unsigned i = 0; i < _subterms.size(); ++i) {
		if ((*_subterms[i]) != (*derived->_subterms[i]))
			return false;
	}

	return true;
}

bool BoundVariable::operator==(const Term& other) const {
	auto derived = dynamic_cast<const BoundVariable*>(&other);
	return derived && _id == derived->_id;
}

bool StateVariable::operator==(const Term& other) const {
	auto derived = dynamic_cast<const StateVariable*>(&other);
	return derived && _variable_id == derived->_variable_id;
}

bool Constant::operator==(const Term& other) const {
	auto derived = dynamic_cast<const Constant*>(&other);
	return derived && _value == derived->_value;
}


std::size_t NestedTerm::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _symbol_id);
	for (const Term* term:_subterms) {
		boost::hash_combine(hash, term->hash_code());
	}
	return hash;
}

std::size_t BoundVariable::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _id);
	return hash;
}

std::size_t StateVariable::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _variable_id);
	return hash;
}

std::size_t Constant::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _value);
	return hash;
}




} } } // namespaces
