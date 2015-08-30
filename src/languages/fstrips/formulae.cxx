
#include <problem_info.hxx>
#include <languages/fstrips/formulae.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>
#include <utils/logging.hxx>

#include <typeinfo>

namespace fs0 { namespace language { namespace fstrips {


// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef RelationalFormula::Symbol AFSymbol;
const std::map<AFSymbol, std::string> RelationalFormula::symbol_to_string{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
const std::map<std::string, AFSymbol> RelationalFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


unsigned AtomicFormula::nestedness() const {
	unsigned max = 0;
	for (Term::cptr subterm:_subterms) max = std::max(max, subterm->nestedness());
	return max;
}

std::vector<Term::cptr> AtomicFormula::flatten() const {
	std::vector<Term::cptr> res;
	for (Term::cptr term:_subterms) {
		auto tmp = term->flatten();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}

bool AtomicFormula::interpret(const PartialAssignment& assignment) const {
	return _satisfied(NestedTerm::interpret_subterms(_subterms, assignment));
}

bool AtomicFormula::interpret(const State& state) const {
	return _satisfied(NestedTerm::interpret_subterms(_subterms, state));
}

std::ostream& RelationalFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " " << RelationalFormula::symbol_to_string.at(symbol()) << " " << *_subterms[1];
	return os;
}


} } } // namespaces
