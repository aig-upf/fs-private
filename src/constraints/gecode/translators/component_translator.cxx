
#include <constraints/gecode/translators/component_translator.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/base.hxx>
#include <constraints/gecode/utils/nested_fluent_iterator.hxx>
#include <problem.hxx>
#include <languages/fstrips/builtin.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace gecode {

typedef fs::RelationalFormula::Symbol AFSymbol;
const std::map<AFSymbol, Gecode::IntRelType> RelationalFormulaTranslator::symbol_to_gecode = {
	{AFSymbol::EQ, Gecode::IRT_EQ}, {AFSymbol::NEQ, Gecode::IRT_NQ}, {AFSymbol::LT, Gecode::IRT_LE},
	{AFSymbol::LEQ, Gecode::IRT_LQ}, {AFSymbol::GT, Gecode::IRT_GR}, {AFSymbol::GEQ, Gecode::IRT_GQ}
};

Gecode::IntRelType RelationalFormulaTranslator::gecode_symbol(fs::RelationalFormula::cptr formula) { return symbol_to_gecode.at(formula->symbol()); }


const std::map<Gecode::IntRelType, Gecode::IntRelType> RelationalFormulaTranslator::operator_inversions = {
	{Gecode::IRT_EQ, Gecode::IRT_EQ}, {Gecode::IRT_NQ, Gecode::IRT_NQ}, {Gecode::IRT_LE, Gecode::IRT_GQ},
	{Gecode::IRT_LQ, Gecode::IRT_GR}, {Gecode::IRT_GR, Gecode::IRT_LQ}, {Gecode::IRT_GQ, Gecode::IRT_LE}
};

Gecode::IntRelType RelationalFormulaTranslator::invert_operator(Gecode::IntRelType op) {
	return operator_inversions.at(op);
}


void ConstantTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) const {
	auto constant = dynamic_cast<fs::Constant::cptr>(term);
	assert(constant);
	translator.registerConstant(constant);
}

void StaticNestedTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) const {
	auto nested = dynamic_cast<fs::NestedTerm::cptr>(term);
	assert(nested);
	translator.registerNestedTerm(nested, type);
}

void ArithmeticTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) const {
	auto nested = dynamic_cast<fs::NestedTerm::cptr>(term);
	assert(nested);
	auto bounds = nested->getBounds();
	translator.registerNestedTerm(nested, type, bounds.first, bounds.second);
}

void ArithmeticTermTranslator::registerConstraints(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) const {
	auto arithmetic_term = dynamic_cast<fs::ArithmeticTerm::cptr>(term);
	assert(arithmetic_term);
	
	FDEBUG("translation", "Registering constraints for arithmetic term " << *term << (type == CSPVariableType::Output ? "'" : ""));

	// Now we assert that the root temporary variable equals the sum of the subterms
	SimpleCSP& csp = translator.getBaseCSP();
	const Gecode::IntVar& result = translator.resolveVariable(arithmetic_term, CSPVariableType::Input, csp);
	Gecode::IntVarArgs operands = translator.resolveVariables(arithmetic_term->getSubterms(), CSPVariableType::Input, csp);
	post(csp, operands, result);
}


void AdditionTermTranslator::post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::linear(csp, getLinearCoefficients(), operands, getRelationType(), result);
}

void SubtractionTermTranslator::post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::linear(csp, getLinearCoefficients(), operands, getRelationType(), result);
}

void MultiplicationTermTranslator::post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
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


void StaticNestedTermTranslator::registerConstraints(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator) const {
	auto stat = dynamic_cast<fs::StaticHeadedNestedTerm::cptr>(term);
	assert(stat);
	
	FDEBUG("translation", "Registering constraints for static nested term " << *stat << (type == CSPVariableType::Output ? "'" : ""));

	SimpleCSP& csp = translator.getBaseCSP();
	
	// Assume we have a static term s(t_1, ..., t_n), where t_i are the subterms.
	// We have registered a termporary variable Z for the whole term, plus temporaries Z_i accounting for each subterm t_i
	// Now we need to post an extensional constraint on all temporary variables <Z_1, Z_2, ..., Z_n, Z> such that
	// the tuples <z_1, ..., z_n, z> satisfying the constraints are exactly those such that z = s(z_1, ..., z_n)
	
	// First compile the variables in the right order (order matters, must be the same than in the tupleset):
	Gecode::IntVarArgs variables = translator.resolveVariables(stat->getSubterms(), CSPVariableType::Input, csp);
	variables << translator.resolveVariable(stat, CSPVariableType::Input, csp);

	// Now compile the tupleset
	Gecode::TupleSet extension = Helper::extensionalize(stat);

	// And finally post the constraint
	Gecode::extensional(csp, variables, extension);
	
	FDEBUG("translation", "Posted extensional constraint:" << print::extensional(variables, extension));
}


void RelationalFormulaTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, GecodeCSPVariableTranslator& translator) const {
	auto condition = dynamic_cast<fs::RelationalFormula::cptr>(formula);
	assert(condition);

	// And register the relation constraint itself
	SimpleCSP& csp = translator.getBaseCSP();
	const Gecode::IntVar& lhs_gec_var = translator.resolveVariable(condition->lhs(), CSPVariableType::Input, csp);
	const Gecode::IntVar& rhs_gec_var = translator.resolveVariable(condition->rhs(), CSPVariableType::Input, csp);
	Gecode::rel(csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
}

void AlldiffGecodeTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, GecodeCSPVariableTranslator& translator) const {
	auto alldiff = dynamic_cast<fs::AlldiffFormula::cptr>(formula);
	assert(alldiff);

	SimpleCSP& csp = translator.getBaseCSP();
	
	Gecode::IntVarArgs variables = translator.resolveVariables(alldiff->getSubterms(), CSPVariableType::Input, csp);
	Gecode::distinct(csp, variables, Gecode::ICL_DOM);
}

void SumGecodeTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, GecodeCSPVariableTranslator& translator) const {
	auto sum = dynamic_cast<fs::SumFormula::cptr>(formula);
	assert(sum);

	SimpleCSP& csp = translator.getBaseCSP();
	Gecode::IntVarArgs variables = translator.resolveVariables(sum->getSubterms(), CSPVariableType::Input, csp);

	// The sum constraint is a particular subcase of gecode's linear constraint with all variables' coefficients set to 1
	// except for the coefficient of the result variable, which is set to -1
	std::vector<int> v_coefficients(variables.size(), 1);
	v_coefficients[variables.size() - 1] = -1; // Last coefficient is a -1, since the last variable of the scope is the element of the sum
	Gecode::IntArgs coefficients(v_coefficients);

	Gecode::linear(csp, coefficients, variables, Gecode::IRT_EQ, 0, Gecode::ICL_DOM);
}

void ExtensionalTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, GecodeCSPVariableTranslator& translator) const {

	SimpleCSP& csp = translator.getBaseCSP();
	// We post an extensional constraint on the CSP variables modeling the value of each of the subterms formula
	Gecode::IntVarArgs variables = translator.resolveVariables(formula->getSubterms(), CSPVariableType::Input, csp);
	Gecode::TupleSet extension = Helper::extensionalize(_symbol);
	Gecode::extensional(csp, variables, extension);
	
	FDEBUG("translation", "Registered a Gecode extensional constraint of arity " << extension.arity() << " and size " << extension.tuples() << " for formula \"" << *formula << ":\n" << print::extensional(variables, extension));
}

} } // namespaces
