
#include <boost/functional/hash.hpp>

#include <problem_info.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <lapkt/tools/logging.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace language { namespace fstrips {

ObjectIdx Term::interpret(const PartialAssignment& assignment) const { return interpret(assignment, Binding::EMPTY_BINDING); }
ObjectIdx Term::interpret(const State& state) const  { return interpret(state, Binding::EMPTY_BINDING); }
VariableIdx Term::interpretVariable(const PartialAssignment& assignment) const { return interpretVariable(assignment, Binding::EMPTY_BINDING); }
VariableIdx Term::interpretVariable(const State& state) const { return interpretVariable(state, Binding::EMPTY_BINDING); }
	
std::ostream& Term::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "<unnamed term>";
	return os;
}

NestedTerm::NestedTerm(const NestedTerm& term) :
	_symbol_id(term._symbol_id),
	_subterms(Utils::clone(term._subterms)),
	_interpreted_subterms(term._interpreted_subterms)
{}


std::vector<const Term*> NestedTerm::all_terms() const {
	std::vector<const Term*> res;
	res.push_back(this);
	for (const Term* term:_subterms) {
		auto tmp = term->all_terms();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}


std::vector<const Term*> NestedTerm::bind_subterms(const std::vector<const Term*>& subterms, const Binding& binding, const ProblemInfo& info, std::vector<ObjectIdx>& constants) {
	assert(constants.empty());
	std::vector<const Term*> result;
	for (auto unprocessed:subterms) {
		auto processed = unprocessed->bind(binding, info);
		result.push_back(processed);
		
		if (const Constant* constant = dynamic_cast<const Constant*>(processed)) {
			constants.push_back(constant->getValue());
		}
	}
	return result;
}

const Term* NestedTerm::create(const std::string& symbol, const std::vector<const Term*>& subterms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	// If the symbol corresponds to an arithmetic term, delegate the creation of the term
	if (ArithmeticTermFactory::isBuiltinTerm(symbol)) return ArithmeticTermFactory::create(symbol, subterms);
	
	unsigned symbol_id = info.getSymbolId(symbol);
	const auto& function = info.getSymbolData(symbol_id);
	if (function.isStatic()) {
		return new UserDefinedStaticTerm(symbol_id, subterms);
	} else {
		return new FluentHeadedNestedTerm(symbol_id, subterms);
	}
}

const Term* NestedTerm::bind(const Binding& binding, const ProblemInfo& info) const {
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> st = bind_subterms(_subterms, binding, info, constant_values);
	
	// We process the 4 different possible cases separately:
	const auto& function = info.getSymbolData(_symbol_id);
	if (function.isStatic() && constant_values.size() == _subterms.size()) { // If all subterms are constants, we can resolve the value of the term schema statically
		for (const auto ptr:st) delete ptr;
		auto value = function.getFunction()(constant_values);
		return info.isBoundedType(function.getCodomainType()) ? new IntConstant(value) : new Constant(value);
	}
	else if (function.isStatic() && constant_values.size() != _subterms.size()) { // We have a statically-headed nested term
		return new UserDefinedStaticTerm(_symbol_id, st);
	}
	else if (!function.isStatic() && constant_values.size() == _subterms.size()) { // If all subterms were constant, and the symbol is fluent, we have a state variable
		VariableIdx id = info.resolveStateVariable(_symbol_id, constant_values);
// 		for (const auto ptr:st) delete ptr;
		return new StateVariable(id, new FluentHeadedNestedTerm(_symbol_id, st));
	}
	else {
		return new FluentHeadedNestedTerm(_symbol_id, st);
	}
}

const Term* FluentHeadedNestedTerm::bind(const Binding& binding, const ProblemInfo& info) const {
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> processed = bind_subterms(_subterms, binding, info, constant_values);
	
	if (constant_values.size() == _subterms.size()) { // If all subterms were constant, and the symbol is fluent, we have a state variable
// 		for (const auto ptr:processed) delete ptr;
		
		VariableIdx id = info.resolveStateVariable(_symbol_id, constant_values);
		return new StateVariable(id, new FluentHeadedNestedTerm(_symbol_id, processed));
	}
	return new FluentHeadedNestedTerm(_symbol_id, processed);
}


const Term* UserDefinedStaticTerm::bind(const Binding& binding, const ProblemInfo& info) const {
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> processed = bind_subterms(_subterms, binding, info, constant_values);
	
	if (constant_values.size() == _subterms.size()) { // If all subterms are constants, we can resolve the value of the term schema statically
		for (const auto ptr:processed) delete ptr;
		
		const auto& function = info.getSymbolData(_symbol_id);
		auto value = function.getFunction()(constant_values);
		return info.isBoundedType(function.getCodomainType()) ? new IntConstant(value) : new Constant(value);
	}
	
	// Otherwise we simply return a user-defined static term with the processed/bound subterms
	return new UserDefinedStaticTerm(_symbol_id, processed);
}


const Term* AxiomaticTerm::bind(const Binding& binding, const ProblemInfo& info) const {
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> processed = bind_subterms(_subterms, binding, info, constant_values);
	
	// We simply return a user-defined static term with the processed/bound subterms
	return clone(processed);
}

AxiomaticTerm* AxiomaticTerm::clone() const { return clone(Utils::clone(_subterms)); }


const Term* ArithmeticTerm::bind(const Binding& binding, const ProblemInfo& info) const {
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> st = bind_subterms(_subterms, binding, info, constant_values);
	
	auto processed = create(st);
	
	if (constant_values.size() == _subterms.size()) { // If all subterms are constants, we can resolve the value of the term schema statically
		auto value = processed->interpret({}, Binding::EMPTY_BINDING);
		delete processed;
		return new IntConstant(value); // Arithmetic terms necessarily involve integer subterms
	}
	else return processed;
}




TypeIdx NestedTerm::getType() const {
	return ProblemInfo::getInstance().getSymbolData(_symbol_id).getCodomainType();
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

std::ostream& NestedTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return printFunction(os, info, _symbol_id, _subterms);
}


StaticHeadedNestedTerm::StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<const Term*>& subterms)
	: NestedTerm(symbol_id, subterms)
{}

ArithmeticTerm::ArithmeticTerm(const std::vector<const Term*>& subterms)
	: StaticHeadedNestedTerm(-1, subterms)
{
	assert(subterms.size() == 2);
}

UserDefinedStaticTerm::UserDefinedStaticTerm(unsigned symbol_id, const std::vector<const Term*>& subterms)
	: StaticHeadedNestedTerm(symbol_id, subterms),
	_function(ProblemInfo::getInstance().getSymbolData(symbol_id))
{}


TypeIdx UserDefinedStaticTerm::getType() const {
	return _function.getCodomainType();
}

std::pair<int, int> UserDefinedStaticTerm::getBounds() const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return info.getTypeBounds(getType());
}


ObjectIdx UserDefinedStaticTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	interpret_subterms(_subterms, assignment, binding, _interpreted_subterms);
	return _function.getFunction()(_interpreted_subterms);
}

ObjectIdx UserDefinedStaticTerm::interpret(const State& state, const Binding& binding) const {
	interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return _function.getFunction()(_interpreted_subterms);
}


AxiomaticTerm::AxiomaticTerm(unsigned symbol_id, const std::vector<const Term*>& subterms)
	: StaticHeadedNestedTerm(symbol_id, subterms)
{}

std::pair<int, int> AxiomaticTerm::getBounds() const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return info.getTypeBounds(getType());
}

ObjectIdx AxiomaticTerm::interpret(const State& state, const Binding& binding) const {
	interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return compute(state, _interpreted_subterms);
}



ObjectIdx FluentHeadedNestedTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	return assignment.at(interpretVariable(assignment, binding));
}

ObjectIdx FluentHeadedNestedTerm::interpret(const State& state, const Binding& binding) const {
	return state.getValue(interpretVariable(state, binding));
}

VariableIdx FluentHeadedNestedTerm::interpretVariable(const PartialAssignment& assignment, const Binding& binding) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	interpret_subterms(_subterms, assignment, binding, _interpreted_subterms);
	VariableIdx variable = info.resolveStateVariable(_symbol_id, _interpreted_subterms);
	return variable;
}
VariableIdx FluentHeadedNestedTerm::interpretVariable(const State& state, const Binding& binding) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	VariableIdx variable = info.resolveStateVariable(_symbol_id, _interpreted_subterms);
	return variable;
}

std::pair<int, int> FluentHeadedNestedTerm::getBounds() const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	auto type = ProblemInfo::getInstance().getSymbolData(_symbol_id).getCodomainType();
	return info.getTypeBounds(type);
}


ObjectIdx StateVariable::interpret(const State& state, const Binding& binding) const {
	return state.getValue(_variable_id);
}

TypeIdx StateVariable::getType() const {
	return ProblemInfo::getInstance().getVariableType(_variable_id);
}

std::pair<int, int> StateVariable::getBounds() const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return info.getVariableBounds(_variable_id);
}

std::ostream& StateVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << info.getVariableName(_variable_id);
	return os;
}

TypeIdx BoundVariable::getType() const { return _type; }

std::pair<int, int> BoundVariable::getBounds() const { return ProblemInfo::getInstance().getTypeBounds(_type); }

const Term* BoundVariable::bind(const Binding& binding, const ProblemInfo& info) const {
	if (!binding.binds(_id)) return clone();
	ObjectIdx value = binding.value(_id);
	return info.isBoundedType(_type) ? new IntConstant(value) : new Constant(value);
}

std::ostream& BoundVariable::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "?" << _id;
	return os;
}


ObjectIdx BoundVariable::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	if (!binding.binds(_id)) throw std::runtime_error("Cannot interpret bound variable without a suitable binding");
	return binding.value(_id);
}

ObjectIdx BoundVariable::interpret(const State& state, const Binding& binding) const {
	if (!binding.binds(_id)) throw std::runtime_error("Cannot interpret bound variable without a suitable binding");
	return binding.value(_id);
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
