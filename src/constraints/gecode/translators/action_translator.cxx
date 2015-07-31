
#include <constraints/gecode/translators/action_translator.hxx>
#include <actions/ground_action.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace gecode {
	
	GecodeActionTranslator::GecodeActionTranslator(const GroundAction& action, SimpleCSP& csp, GecodeCSPTranslator& translator)
		: GecodeBaseTranslator(csp, translator), _action(action), _temp_output_variables(0)
	{}
	
	void GecodeActionTranslator::registerVariables(Gecode::IntVarArgs& variables) {
		// Register first the planning state variables
		for (VariableIdx var:_action.getAllRelevantVariables()) {
			Helper::registerPlanningVariable(_csp, var, CSPVariableType::Input, variables, _translator);
		}

		for (VariableIdx var:_action.getAffectedVariables()) {
			Helper::registerPlanningVariable(_csp, var, CSPVariableType::Output, variables, _translator);
		}
		
		// Register now the temporary variables
		registerTempVariables(_action.getConditions(), variables);
		registerTempVariables(_action.getEffects(), variables);
	}
	
	
	void GecodeActionTranslator::registerTempVariables(const fs::ActionEffect::cptr effect, Gecode::IntVarArgs& variables) {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		
		// Register first the RHS variables
		registerTempVariables(effect->rhs, variables);
		
		// We do not want to parse the root of the LHS, as it gives rise to a different type of CSP variable
		// Thus, we only parse the subterms of the LHS, if available
		auto _nested = dynamic_cast<NestedTerm::cptr>(effect->lhs);
		if (_nested) {
			registerTempVariables(_nested->getSubterms(), variables);
		}
		
		// Finally, the LHS root gets registered separately, to ensure it gives rise to a distinct temporary variable,
		// even if the subterm itself was already present in some other part of the action.
		TypeIdx type = info.getFunctionData(_nested->getSymbolId()).getCodomainType();
		Helper::registerTemporaryOutputVariable(_csp, _temp_output_variables, variables, _translator, type);
		++_temp_output_variables;
	}
	
	void GecodeActionTranslator::registerTempVariables(const std::vector<fs::ActionEffect::cptr>& effects, Gecode::IntVarArgs& variables) {
		for (const auto effect:effects) registerTempVariables(effect, variables);
	}
	
	
	
	
	void GecodeActionTranslator::registerConstraints() const {
		registerConstraints(_action.getConditions());
		registerConstraints(_action.getEffects());
	}
	
	
	void GecodeActionTranslator::registerConstraints(const fs::ActionEffect::cptr effect) const {
		assert(0);
		// TODO FINISH
// 		Helper::translateEffects(*base_csp, translator, action.getEffects()); // Action effects
	}
	
	void GecodeActionTranslator::registerConstraints(const std::vector<fs::ActionEffect::cptr>& effects) const {
		for (const auto effect:effects) registerConstraints(effect);
	}

} } // namespaces
