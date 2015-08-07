
#include <languages/fstrips/builtin.hxx>
#include <problem_info.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

namespace fs0 { namespace language { namespace fstrips {


StaticHeadedNestedTerm::cptr BuiltinTermFactory::create(const std::string& symbol, const std::vector<const Term*>& subterms) {
	if (symbol == "+")      return new AdditionTerm(subterms);
	else if (symbol == "-") return new SubtractionTerm(subterms);
	else if (symbol == "*") return new MultiplicationTerm(subterms);
	return nullptr;
}



AdditionTerm::AdditionTerm(const std::vector<Term::cptr>& subterms)
	: StaticHeadedNestedTerm(-1, subterms)
{
	assert(subterms.size() == 2);
}

ObjectIdx AdditionTerm::interpret(const PartialAssignment& assignment) const {
	return _subterms[0]->interpret(assignment) + _subterms[1]->interpret(assignment);
}

ObjectIdx AdditionTerm::interpret(const State& state) const {
	return _subterms[0]->interpret(state) + _subterms[1]->interpret(state);
}

std::pair<int, int> AdditionTerm::getBounds() const {
	auto min = _subterms[0]->getBounds().first + _subterms[1]->getBounds().first; 
	auto max = _subterms[0]->getBounds().second + _subterms[1]->getBounds().second; 
	return std::make_pair(min, max);
}

std::ostream& AdditionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " + " << *_subterms[1];
	return os;
}

SubtractionTerm::SubtractionTerm(const std::vector<Term::cptr>& subterms)
	: StaticHeadedNestedTerm(-1, subterms)
{
	assert(subterms.size() == 2);
}

ObjectIdx SubtractionTerm::interpret(const PartialAssignment& assignment) const {
	return _subterms[0]->interpret(assignment) - _subterms[1]->interpret(assignment);
}

ObjectIdx SubtractionTerm::interpret(const State& state) const {
	return _subterms[0]->interpret(state) - _subterms[1]->interpret(state);
}

std::pair<int, int> SubtractionTerm::getBounds() const {
	auto min = _subterms[0]->getBounds().first + _subterms[1]->getBounds().second; 
	auto max = _subterms[0]->getBounds().second + _subterms[1]->getBounds().first; 
	return std::make_pair(min, max);
}

std::ostream& SubtractionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " - " << *_subterms[1];
	return os;
}


MultiplicationTerm::MultiplicationTerm(const std::vector<Term::cptr>& subterms)
	: StaticHeadedNestedTerm(-1, subterms)
{
	assert(subterms.size() == 2);
}

ObjectIdx MultiplicationTerm::interpret(const PartialAssignment& assignment) const {
	return _subterms[0]->interpret(assignment) * _subterms[1]->interpret(assignment);
}

ObjectIdx MultiplicationTerm::interpret(const State& state) const {
	return _subterms[0]->interpret(state) * _subterms[1]->interpret(state);
}

std::pair<int, int> MultiplicationTerm::getBounds() const {
	auto min = _subterms[0]->getBounds().first * _subterms[1]->getBounds().first; 
	auto max = _subterms[0]->getBounds().second * _subterms[1]->getBounds().second; 
	return std::make_pair(min, max);
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

bool AlldiffFormula::_satisfied(const ObjectIdxVector& values) const {
	std::set<ObjectIdx> distinct;
	for (ObjectIdx val:values) {
		auto res = distinct.insert(val);
		if (!res.second) return false; // We found a duplicate, hence the formula is false
	}
	return true;
}



bool SumFormula::_satisfied(const ObjectIdxVector& values) const {
	int expected_sum_value = values.back();
	int total_sum  = std::accumulate(values.begin(), values.end(), 0);
	return expected_sum_value*2 == total_sum;
}


} } } // namespaces
