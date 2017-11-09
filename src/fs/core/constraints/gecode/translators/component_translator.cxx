
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/constraints/gecode/translators/component_translator.hxx>
#include <fs/core/constraints/gecode/gecode_csp.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>
#include <fs/core/constraints/gecode/helper.hxx>
#include <fs/core/constraints/gecode/handlers/base_csp.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/utils//printers/gecode.hxx>

namespace fs0 { namespace gecode {

//! A helper data structure to help translate language relational symbols to gecode relational operators
typedef fs::RelationalFormula::Symbol AFSymbol;
const std::map<AFSymbol, Gecode::IntRelType> symbol_to_gecode = {
	{AFSymbol::EQ, Gecode::IRT_EQ}, {AFSymbol::NEQ, Gecode::IRT_NQ}, {AFSymbol::LT, Gecode::IRT_LE},
	{AFSymbol::LEQ, Gecode::IRT_LQ}, {AFSymbol::GT, Gecode::IRT_GR}, {AFSymbol::GEQ, Gecode::IRT_GQ}
};

Gecode::IntRelType RelationalFormulaTranslator::gecode_symbol(const fs::RelationalFormula* formula) { return symbol_to_gecode.at(formula->symbol()); }


const std::map<Gecode::IntRelType, Gecode::IntRelType> RelationalFormulaTranslator::operator_inversions = {
	{Gecode::IRT_EQ, Gecode::IRT_EQ}, {Gecode::IRT_NQ, Gecode::IRT_NQ}, {Gecode::IRT_LE, Gecode::IRT_GQ},
	{Gecode::IRT_LQ, Gecode::IRT_GR}, {Gecode::IRT_GR, Gecode::IRT_LQ}, {Gecode::IRT_GQ, Gecode::IRT_LE}
};

Gecode::IntRelType RelationalFormulaTranslator::invert_operator(Gecode::IntRelType op) {
	return operator_inversions.at(op);
}


void ConstantTermTranslator::registerVariables(const fs::Term* term, CSPTranslator& translator) const {
	auto constant = dynamic_cast<const fs::Constant*>(term);
	assert(constant);
	translator.registerConstant(constant);
}

void BoundVariableTermTranslator::registerVariables(const fs::Term* term, CSPTranslator& translator) const {
	auto variable = dynamic_cast<const fs::BoundVariable*>(term);
	assert(variable);
	translator.registerExistentialVariable(variable);
}

void StaticNestedTermTranslator::registerVariables(const fs::Term* term, CSPTranslator& translator) const {
	auto nested = dynamic_cast<const fs::NestedTerm*>(term);
	assert(nested);
	translator.registerNestedTerm(nested);
}

void ArithmeticTermTranslator::registerVariables(const fs::Term* term, CSPTranslator& translator) const {
	auto nested = dynamic_cast<const fs::NestedTerm*>(term);
	assert(nested);
	auto bounds = fs::bounds(*nested);
	translator.registerNestedTerm(nested, bounds.first, bounds.second);
}

void ArithmeticTermTranslator::registerConstraints(const fs::Term* term, CSPTranslator& translator) const {
	auto arithmetic_term = dynamic_cast<const fs::ArithmeticTerm*>(term);
	assert(arithmetic_term);

	LPT_DEBUG("translation", "Registering constraints for arithmetic term " << *term);

	// Now we assert that the root temporary variable equals the sum of the subterms
	GecodeCSP& csp = translator.getBaseCSP();
	const Gecode::IntVar& result = translator.resolveVariable(arithmetic_term, csp);
	Gecode::IntVarArgs operands = translator.resolveVariables(arithmetic_term->getSubterms(), csp);
	post(csp, operands, result);
}


void AdditionTermTranslator::post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::linear(csp, getLinearCoefficients(), operands, getRelationType(), result);
}

void SubtractionTermTranslator::post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::linear(csp, getLinearCoefficients(), operands, getRelationType(), result);
}

void MultiplicationTermTranslator::post(GecodeCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::mult(csp, operands[0], operands[1], result);
}

Gecode::IntArgs AdditionTermTranslator::getLinearCoefficients() const {
	std::vector<int> coefficients{1, 1};
	return Gecode::IntArgs(coefficients);
}

Gecode::IntArgs SubtractionTermTranslator::getLinearCoefficients() const {
	std::vector<int> coefficients{1, -1};
	return Gecode::IntArgs(coefficients);
}

Gecode::IntArgs MultiplicationTermTranslator::getLinearCoefficients() const {
	std::vector<int> coefficients{1, -1};
	return Gecode::IntArgs(coefficients);
}


void StaticNestedTermTranslator::registerConstraints(const fs::Term* term, CSPTranslator& translator) const {
	auto stat = dynamic_cast<const fs::StaticHeadedNestedTerm*>(term);
	assert(stat);

	LPT_DEBUG("translation", "Registering constraints for static nested term " << *stat);

	GecodeCSP& csp = translator.getBaseCSP();

	// Assume we have a static term s(t_1, ..., t_n), where t_i are the subterms.
	// We have registered a temporary variable Z for the whole term, plus temporaries Z_i accounting for each subterm t_i
	// Now we need to post an extensional constraint on all temporary variables <Z_1, Z_2, ..., Z_n, Z> such that
	// the tuples <z_1, ..., z_n, z> satisfying the constraints are exactly those such that z = s(z_1, ..., z_n)

	// First compile the variables in the right order (order matters, must be the same than in the tupleset):
	Gecode::IntVarArgs variables = translator.resolveVariables(stat->getSubterms(), csp);
	variables << translator.resolveVariable(stat, csp);

	// Now compile the tupleset
	Gecode::TupleSet extension = Helper::extensionalize(stat);

	// And finally post the constraint
	Gecode::extensional(csp, variables, extension);
	LPT_EDEBUG("translation", "Posted extensional constraint:" << print::extensional(variables, extension));
}


void RelationalFormulaTranslator::registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const {
	auto condition = dynamic_cast<const fs::RelationalFormula*>(formula);
	assert(condition);

	// And register the relation constraint itself
	GecodeCSP& csp = translator.getBaseCSP();
	const Gecode::IntVar& lhs_gec_var = translator.resolveVariable(condition->lhs(), csp);
	const Gecode::IntVar& rhs_gec_var = translator.resolveVariable(condition->rhs(), csp);
	Gecode::rel(csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
}

void AlldiffGecodeTranslator::registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const {
	auto alldiff = dynamic_cast<const fs::AlldiffFormula*>(formula);
	assert(alldiff);

	GecodeCSP& csp = translator.getBaseCSP();

	Gecode::IntVarArgs variables = translator.resolveVariables(alldiff->getSubterms(), csp);
	Gecode::distinct(csp, variables);
}

void SumGecodeTranslator::registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const {
	auto sum = dynamic_cast<const fs::SumFormula*>(formula);
	assert(sum);

	GecodeCSP& csp = translator.getBaseCSP();
	Gecode::IntVarArgs variables = translator.resolveVariables(sum->getSubterms(), csp);

	// The sum constraint is a particular subcase of gecode's linear constraint with all variables' coefficients set to 1
	// except for the coefficient of the result variable, which is set to -1
	std::vector<int> v_coefficients(variables.size(), 1);
	v_coefficients[variables.size() - 1] = -1; // Last coefficient is a -1, since the last variable of the scope is the element of the sum
	Gecode::IntArgs coefficients(v_coefficients);

	Gecode::linear(csp, coefficients, variables, Gecode::IRT_EQ, 0);
}


void NValuesGecodeTranslator::registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const {
	auto nvalues = dynamic_cast<const fs::NValuesFormula*>(formula);
	assert(nvalues);

	GecodeCSP& csp = translator.getBaseCSP();

	std::vector<const fs::Term*> st = nvalues->getSubterms();
	assert(st.size()>1);

	std::vector<const fs::Term*> elements(st.begin(), st.end()-1);
	Gecode::IntVarArgs elements_var = translator.resolveVariables(elements, csp);
	auto num_vals = translator.resolveVariable(st.back(), csp);;
	Gecode::nvalues(csp, elements_var, Gecode::IRT_EQ, num_vals);
}


void ExtensionalTranslator::registerConstraints(const fs::AtomicFormula* formula, CSPTranslator& translator) const {

	GecodeCSP& csp = translator.getBaseCSP();
	// We post an extensional constraint on the CSP variables modeling the value of each of the subterms formula
	Gecode::IntVarArgs variables = translator.resolveVariables(formula->getSubterms(), csp);
	Gecode::TupleSet extension = Helper::extensionalize(formula);
	Gecode::extensional(csp, variables, extension);

	LPT_DEBUG("translation", "Registered a Gecode extensional constraint of arity " << extension.arity() << " and size " << extension.tuples() << " for formula \"" << *formula << ":\n" << print::extensional(variables, extension));
}

} } // namespaces
