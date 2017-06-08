
#include <numeric>

#include <unordered_set>

#include <languages/fstrips/builtin.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

namespace fs0 { namespace language { namespace fstrips {


const StaticHeadedNestedTerm* ArithmeticTermFactory::create(const std::string& symbol, const std::vector<const Term*>& subterms) {
	if (symbol == "+")      return new AdditionTerm(subterms);
	else if (symbol == "-") return new SubtractionTerm(subterms);
	else if (symbol == "*") return new MultiplicationTerm(subterms);
	return nullptr;
}


AdditionTerm::AdditionTerm(const std::vector<const Term*>& subterms)
	: ArithmeticTerm(subterms) {}

object_id AdditionTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	int x = value<int>(_subterms[0]->interpret(assignment, binding), ObjectTable::EMPTY_TABLE);
	int y = value<int>(_subterms[1]->interpret(assignment, binding), ObjectTable::EMPTY_TABLE);
	return make_obj(x+y);
}

object_id AdditionTerm::interpret(const State& state, const Binding& binding) const {
	int x = value<int>(_subterms[0]->interpret(state, binding), ObjectTable::EMPTY_TABLE);
	int y = value<int>(_subterms[1]->interpret(state, binding), ObjectTable::EMPTY_TABLE);
	return make_obj(x+y);
}


std::ostream& AdditionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " + " << *_subterms[1];
	return os;
}

SubtractionTerm::SubtractionTerm(const std::vector<const Term*>& subterms)
	: ArithmeticTerm(subterms) {}

object_id SubtractionTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	int x = value<int>(_subterms[0]->interpret(assignment, binding), ObjectTable::EMPTY_TABLE);
	int y = value<int>(_subterms[1]->interpret(assignment, binding), ObjectTable::EMPTY_TABLE);
	return make_obj(x-y);
}

object_id SubtractionTerm::interpret(const State& state, const Binding& binding) const {
	int x = value<int>(_subterms[0]->interpret(state, binding), ObjectTable::EMPTY_TABLE);
	int y = value<int>(_subterms[1]->interpret(state, binding), ObjectTable::EMPTY_TABLE);
	return make_obj(x-y);
}

std::ostream& SubtractionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " - " << *_subterms[1];
	return os;
}

MultiplicationTerm::MultiplicationTerm(const std::vector<const Term*>& subterms)
	: ArithmeticTerm(subterms) {}

object_id MultiplicationTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	int x = value<int>(_subterms[0]->interpret(assignment, binding), ObjectTable::EMPTY_TABLE);
	int y = value<int>(_subterms[1]->interpret(assignment, binding), ObjectTable::EMPTY_TABLE);
	return make_obj(x*y);
}

object_id MultiplicationTerm::interpret(const State& state, const Binding& binding) const {
	int x = value<int>(_subterms[0]->interpret(state, binding), ObjectTable::EMPTY_TABLE);
	int y = value<int>(_subterms[1]->interpret(state, binding), ObjectTable::EMPTY_TABLE);
	return make_obj(x*y);
}


std::ostream& MultiplicationTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " * " << *_subterms[1];
	return os;
}

AlldiffFormula::AlldiffFormula(const AlldiffFormula& formula) : AlldiffFormula(Utils::clone(formula._subterms)) {}

bool AlldiffFormula::_satisfied(const std::vector<object_id>& values) const {
	std::unordered_set<object_id> distinct;
	for (object_id val:values) {
		auto res = distinct.insert(val);
		if (!res.second) return false; // We found a duplicate, hence the formula is false
	}
	return true;
}

SumFormula::SumFormula(const SumFormula& formula) : SumFormula(Utils::clone(formula._subterms)) {}

bool SumFormula::_satisfied(const std::vector<object_id>& values) const {
	// sum(x_1, ..., x_n) meaning x_1 + ... + x_{n-1} = x_n
	assert(values.size() > 1);
	
	int accum = 0;
	// Accumulate all values except the last one
	for (unsigned i = 0; i < values.size()-1; ++i) {
		accum += value<int>(values[i], ObjectTable::EMPTY_TABLE);
	}
	return accum == value<int>(values.back(), ObjectTable::EMPTY_TABLE);
}

NValuesFormula::NValuesFormula(const NValuesFormula& formula) : NValuesFormula(Utils::clone(formula._subterms)) {}

bool NValuesFormula::_satisfied(const std::vector<object_id>& values) const {
	// nvalues(x_1, ..., x_n) meaning there are exactly x_n different values among variables <x_1, ... x_{n-1}>
	assert(values.size() > 1);
	assert(int(values[values.size()-1]) > 0);
	int n = value<int>(values.back(), ObjectTable::EMPTY_TABLE);
	std::unordered_set<object_id> distinct;
	for (unsigned i = 0; i < values.size() - 1; ++i) {
		distinct.insert(values[i]);
	}
	return distinct.size() == (std::size_t) n;
}

} } } // namespaces
