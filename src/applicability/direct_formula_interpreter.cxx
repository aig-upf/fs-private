
#include <applicability/direct_formula_interpreter.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {

DirectFormulaInterpreter::DirectFormulaInterpreter(const fs::Formula::cptr formula) :
	_formula(formula)
{}

bool DirectFormulaInterpreter::satisfied(const State& state) const {
	return _formula->interpret(state);
}

} // namespaces
