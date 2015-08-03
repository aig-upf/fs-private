
#include <constraints/gecode/translators/formula_translator.hxx>
#include <actions/ground_action.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace gecode {
	
	GecodeFormulaTranslator::GecodeFormulaTranslator(const std::vector<fs::AtomicFormula::cptr>& conditions, const VariableIdxVector& relevant, SimpleCSP& csp, GecodeCSPVariableTranslator& translator)
		: GecodeBaseTranslator(csp, translator), _conditions(conditions), _relevant(relevant) {}

	void GecodeFormulaTranslator::registerVariables(Gecode::IntVarArgs& variables) {
		
		for (VariableIdx var:_relevant) {
			Helper::registerPlanningVariable(_csp, var, CSPVariableType::Input, variables, _translator);
		}
		
		// Register now the temporary variables
		registerTempVariables(_conditions, variables);
	}
	
	void GecodeFormulaTranslator::registerConstraints() const {
		registerConstraints(_conditions);
	}

} } // namespaces
