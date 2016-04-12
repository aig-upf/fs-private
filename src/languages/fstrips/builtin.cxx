
#include <languages/fstrips/builtin.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

namespace fs0 { namespace language { namespace fstrips {


StaticHeadedNestedTerm::cptr ArithmeticTermFactory::create(const std::string& symbol, const std::vector<const Term*>& subterms) {
	if (symbol == "+")      return new AdditionTerm(subterms);
	else if (symbol == "-") return new SubtractionTerm(subterms);
	else if (symbol == "*") return new MultiplicationTerm(subterms);
	return nullptr;
}


AdditionTerm::AdditionTerm(const std::vector<Term::cptr>& subterms)
	: ArithmeticTerm(subterms) {}

ObjectIdx AdditionTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	return _subterms[0]->interpret(assignment, binding) + _subterms[1]->interpret(assignment, binding);
}

ObjectIdx AdditionTerm::interpret(const State& state, const Binding& binding) const {
	return _subterms[0]->interpret(state, binding) + _subterms[1]->interpret(state, binding);
}

std::pair<int, int> AdditionTerm::getBounds() const {
	// see https://en.wikipedia.org/wiki/Interval_arithmetic
	auto b0 = _subterms[0]->getBounds();
	auto b1 = _subterms[1]->getBounds();
	auto min = b0.first + b1.first; 
	auto max = b0.second + b1.second; 
	return std::make_pair(min, max);
}

std::ostream& AdditionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " + " << *_subterms[1];
	return os;
}

SubtractionTerm::SubtractionTerm(const std::vector<Term::cptr>& subterms)
	: ArithmeticTerm(subterms) {}

ObjectIdx SubtractionTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	return _subterms[0]->interpret(assignment, binding) - _subterms[1]->interpret(assignment, binding);
}

ObjectIdx SubtractionTerm::interpret(const State& state, const Binding& binding) const {
	return _subterms[0]->interpret(state, binding) - _subterms[1]->interpret(state, binding);
}

std::pair<int, int> SubtractionTerm::getBounds() const {
	// see https://en.wikipedia.org/wiki/Interval_arithmetic
	auto b0 = _subterms[0]->getBounds();
	auto b1 = _subterms[1]->getBounds();
	auto min = b0.first - b1.second; 
	auto max = b0.second - b1.first; 
	return std::make_pair(min, max);
}

std::ostream& SubtractionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " - " << *_subterms[1];
	return os;
}


MultiplicationTerm::MultiplicationTerm(const std::vector<Term::cptr>& subterms)
	: ArithmeticTerm(subterms) {}

ObjectIdx MultiplicationTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	return _subterms[0]->interpret(assignment, binding) * _subterms[1]->interpret(assignment, binding);
}

ObjectIdx MultiplicationTerm::interpret(const State& state, const Binding& binding) const {
	return _subterms[0]->interpret(state, binding) * _subterms[1]->interpret(state, binding);
}

std::pair<int, int> MultiplicationTerm::getBounds() const {
	// see https://en.wikipedia.org/wiki/Interval_arithmetic
	auto b0 = _subterms[0]->getBounds();
	auto b1 = _subterms[1]->getBounds();
	auto all{b0.first * b1.first, b0.first * b1.second, b0.second * b1.first, b0.second * b1.second};
	auto minmax = std::minmax_element(all.begin(), all.end());
	return std::make_pair(*minmax.first, *minmax.second);
}

std::ostream& MultiplicationTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " * " << *_subterms[1];
	return os;
}

std::ostream& ExternallyDefinedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << name() << "(";
	for (const auto ptr:_subterms) os << *ptr << ", ";
	os << ")";
	return os;
}

AlldiffFormula::AlldiffFormula(const AlldiffFormula& formula) : AlldiffFormula(Utils::clone(formula._subterms)) {}

bool AlldiffFormula::_satisfied(const ObjectIdxVector& values) const {
	std::set<ObjectIdx> distinct;
	for (ObjectIdx val:values) {
		auto res = distinct.insert(val);
		if (!res.second) return false; // We found a duplicate, hence the formula is false
	}
	return true;
}

SumFormula::SumFormula(const SumFormula& formula) : SumFormula(Utils::clone(formula._subterms)) {}

bool SumFormula::_satisfied(const ObjectIdxVector& values) const {
	// sum(x_1, ..., x_n) meaning x_1 + ... + x_{n-1} = x_n
	assert(values.size() > 1);
	int expected_sum = values.back();
	int addends_sum  = std::accumulate(values.begin(), values.end() - 1, 0);
	return addends_sum == expected_sum;
}


} } } // namespaces
