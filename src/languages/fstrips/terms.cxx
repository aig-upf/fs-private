
#include <problem_info.hxx>
#include <languages/fstrips/terms.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>
#include <utils/logging.hxx>

#include <typeinfo>

namespace fs0 { namespace language { namespace fstrips {


//! A helper to interpret a vector of terms
template <typename T>
ObjectIdxVector NestedTerm::interpret_subterms(const std::vector<Term::cptr>& subterms, const T& assignment) {
	ObjectIdxVector interpreted;
	for (Term::cptr subterm:subterms) {
		interpreted.push_back(subterm->interpret(assignment));
	}
	return interpreted;
}

std::ostream& LogicalElement::print(std::ostream& os) const { return print(os, Problem::getInfo()); }

std::ostream& Term::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "<unnamed term>";
	return os;
}

std::vector<Term::cptr> NestedTerm::flatten() const {
	std::vector<Term::cptr> res;
	res.push_back(this);
	for (Term::cptr term:_subterms) {
		auto tmp = term->flatten();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}


std::ostream& NestedTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return printFunction(os, info, _symbol_id, _subterms);
}


StaticHeadedNestedTerm::StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
	: NestedTerm(symbol_id, subterms)
{}

UserDefinedStaticTerm::UserDefinedStaticTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
	: StaticHeadedNestedTerm(symbol_id, subterms),
	_function(Problem::getInfo().getFunctionData(symbol_id))
{}

std::pair<int, int> UserDefinedStaticTerm::getBounds() const {
	const ProblemInfo& info = Problem::getInfo();
	auto type = _function.getCodomainType();
	return info.getTypeBounds(type);
}

ObjectIdx UserDefinedStaticTerm::interpret(const PartialAssignment& assignment) const {
	return _function.getFunction()(interpret_subterms(_subterms, assignment));
}

ObjectIdx UserDefinedStaticTerm::interpret(const State& state) const {
	return _function.getFunction()(interpret_subterms(_subterms, state));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const PartialAssignment& assignment) const {
	return assignment.at(interpretVariable(assignment));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const State& state) const {
	return state.getValue(interpretVariable(state));
}

VariableIdx FluentHeadedNestedTerm::interpretVariable(const PartialAssignment& assignment) const {
	const ProblemInfo& info = Problem::getInfo();
	VariableIdx variable = info.resolveStateVariable(_symbol_id, interpret_subterms(_subterms, assignment));
	return variable;
}
VariableIdx FluentHeadedNestedTerm::interpretVariable(const State& state) const {
	const ProblemInfo& info = Problem::getInfo();
	VariableIdx variable = info.resolveStateVariable(_symbol_id, interpret_subterms(_subterms, state));
	return variable;
}

std::pair<int, int> FluentHeadedNestedTerm::getBounds() const {
	const ProblemInfo& info = Problem::getInfo();
	auto type = Problem::getInfo().getFunctionData(_symbol_id).getCodomainType();
	return info.getTypeBounds(type);
}

ObjectIdx StateVariable::interpret(const State& state) const {
	return state.getValue(_variable_id);
}

std::pair<int, int> StateVariable::getBounds() const {
	const ProblemInfo& info = Problem::getInfo();
	return info.getVariableBounds(_variable_id);
}

std::ostream& StateVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.getVariableName(_variable_id);
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
	for (const Term::cptr term:_subterms) {
		boost::hash_combine(hash, term->hash_code());
	}
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
