
#pragma once
#include <constraints/gecode/translators/base_translator.hxx>


namespace fs0 {
	class GroundAction;
}

namespace fs0 { namespace gecode {

class GecodeActionTranslator : public GecodeBaseTranslator {
public:
	GecodeActionTranslator(const GroundAction& action, SimpleCSP& csp, GecodeCSPTranslator& translator);

	//! The translator can optionally register any number of (probably temporary) CSP variables.
	void registerVariables(Gecode::IntVarArgs& variables);
	
	//! The translator can register any number of CSP constraints
	void registerConstraints() const;

protected:
	//! The action to be translated
	const GroundAction& _action;
	
	//! An id-counter for temporary variables that substitute LHS terms in assignments
	unsigned _temp_output_variables;
	
	
	using GecodeBaseTranslator::registerTempVariables;
	using GecodeBaseTranslator::registerConstraints;
	
	// Variable registration methods
	void registerTempVariables(const fs::ActionEffect::cptr effect, Gecode::IntVarArgs& variables);
	void registerTempVariables(const std::vector<fs::ActionEffect::cptr>& effects, Gecode::IntVarArgs& variables);
	
	// Constraint registration methods
	void registerConstraints(const fs::ActionEffect::cptr effect) const;
	void registerConstraints(const std::vector<fs::ActionEffect::cptr>& effects) const;
};

} } // namespaces
