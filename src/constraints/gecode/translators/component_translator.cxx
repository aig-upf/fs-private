
#include <constraints/gecode/translators/component_translator.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <problem.hxx>
#include <languages/fstrips/builtin.hxx>

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
	
	
	
	
	
	
void ConstantTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto constant = dynamic_cast<fs::Constant::cptr>(term);
	assert(constant);
	translator.registerConstant(constant, csp, variables);
}

void StateVariableTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto variable = dynamic_cast<fs::StateVariable::cptr>(term);
	assert(variable);
	translator.registerStateVariable(variable, root_type, csp, variables);
}

void NestedTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType root_type, CSPVariableType children_type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto nested = dynamic_cast<fs::NestedTerm::cptr>(term);
	assert(nested);
	
	// If the subterm occurs somewhere else in the action / formula, it might have already been parsed and registered,
	// in which case we do NOT want to register it again
	if (translator.isRegistered(nested, root_type)) return;
	
	// We first parse and register the subterms recursively
	GecodeCSPHandler::registerTermVariables(nested->getSubterms(), children_type, csp, translator, variables);
	
	// And now register the CSP variable corresponding to the current term
	translator.registerNestedTerm(nested, root_type, csp, variables);
}

void StaticNestedTermTranslator::registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto stat = dynamic_cast<fs::StaticHeadedNestedTerm::cptr>(term);
	assert(stat);
	const std::vector<fs::Term::cptr>& subterms = stat->getSubterms();
	
	// First we register recursively the constraints of the subterms
	GecodeCSPHandler::registerTermConstraints(subterms, csp, translator);
	
	/*
	VariableIdxVector scope = stat->computeScope();
	Gecode::TupleSet extension = Helper::extensionalize(stat, scope);
*/
	
	// TODO - This is incorrect - we need to compile static fluents differently
	// We need to use both state variables and temporary variables
	assert(0); 
/*		
	IntVarArgs ordered_variables;
	
	auto output_var = _translator.resolveFunction(csp, output_var_id, CSPVariableType::Temporary);
	
	// TODO - Add additional variables corresponding to temporaries...
	
	ordered_variables << output_var;
	
	Gecode::extensional(csp, ordered_variables, extension); // Order matters - variable order must be the same than in the tupleset
*/
}

void FluentNestedTermTranslator::registerConstraints(const fs::Term::cptr term, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto fluent = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term);
	assert(fluent);
	const std::vector<fs::Term::cptr>& subterms = fluent->getSubterms();
	
	// First we register recursively the constraints of the subterms
	GecodeCSPHandler::registerTermConstraints(subterms, csp, translator);
	
	
	if (subterms.size() > 1) throw std::runtime_error("Nested subterms of arity > 1 not yet implemented");
	assert(subterms.size() == 1); // Cannot be 0, or we'd have instead a StateVariable term
	fs::Term::cptr st = subterms[0];
	
	/*
	IntVar index_var;
	auto it = _temp_variables.find(st);
	if (it != _temp_variables.end()) {
		index_var = _translator.resolveVariable(csp, it->second.getVariableId(), CSPVariableType::Temporary);
	} else {
		auto _var = dynamic_cast<StateVariable::cptr>(st);
		assert(_var);
		index_var = _translator.resolveVariable(csp, _var->getValue(), CSPVariableType::Temporary);
	}
	
	Gecode::IntVarArgs function_vars = _translator.resolveFunction(csp, fluent->getSymbolId(), CSPVariableType::Input);
	auto output_var = _translator.resolveFunction(csp, output_var_id, CSPVariableType::Temporary);
	*/
	assert(false); // TODO - REGISTER the full element constraint, with reindexing through an extensional constraint
	// TODO - This will probably also require to index more variables
	
// 		Gecode::element(csp, function_vars, index_var, output_var);
}






void AtomicFormulaTranslator::registerVariables(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	// We simply need to recursively register the variables of each subterm
	for (const Term::cptr subterm:formula->getSubterms()) {
		GecodeCSPHandler::registerTermVariables(subterm, CSPVariableType::Input, csp, translator, variables); // Formula variables are always input variables
	}
}

void AtomicFormulaTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	// We simply need to recursively register the constraints of each subterm
	for (const Term::cptr subterm:formula->getSubterms()) {
		GecodeCSPHandler::registerTermConstraints(subterm, csp, translator);
	}
}

void RelationalFormulaTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto condition = dynamic_cast<fs::RelationalFormula::cptr>(formula);
	assert(condition);
	
	// Register possible nested constraints recursively by calling the parent registrar
	AtomicFormulaTranslator::registerConstraints(formula, csp, translator);
	
	// And register the relation constraint itself
	const Gecode::IntVar& lhs_gec_var = translator.resolveVariable(condition->lhs(), CSPVariableType::Input, csp);
	const Gecode::IntVar& rhs_gec_var = translator.resolveVariable(condition->rhs(), CSPVariableType::Input, csp);
	Gecode::rel(csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
}

void AlldiffGecodeTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto alldiff = dynamic_cast<fs::AlldiffFormula::cptr>(formula);
	assert(alldiff);
	
	// Register possible nested constraints recursively by calling the parent registrar
	AtomicFormulaTranslator::registerConstraints(formula, csp, translator);
	
	Gecode::IntVarArgs variables = translator.resolveVariables(alldiff->getSubterms(), CSPVariableType::Input, csp);
	Gecode::distinct(csp, variables);
}

void SumGecodeTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto sum = dynamic_cast<fs::SumFormula::cptr>(formula);
	assert(sum);
	
	// Register possible nested constraints recursively by calling the parent registrar
	AtomicFormulaTranslator::registerConstraints(formula, csp, translator);
	
	
	Gecode::IntVarArgs variables = translator.resolveVariables(sum->getSubterms(), CSPVariableType::Input, csp);
	
	// The sum constraint is a particular subcase of gecode's linear constraint with all variables' coefficients set to 1
	// except for the coefficient of the result variable, which is set to -1
	std::vector<int> v_coefficients(variables.size(), 1);
	v_coefficients[variables.size() - 1] = -1; // Last coefficient is a -1, since the last variable of the scope is the element of the sum
	Gecode::IntArgs coefficients(v_coefficients);
	
	Gecode::linear(csp, coefficients, variables, Gecode::IRT_EQ, 0);
}


} } // namespaces
