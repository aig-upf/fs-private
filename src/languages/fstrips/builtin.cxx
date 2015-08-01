
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

std::ostream& MultiplicationTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " * " << *_subterms[1];
	return os;
}

} } } // namespaces
