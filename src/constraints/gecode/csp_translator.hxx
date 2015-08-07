
#pragma once

#include <unordered_map>


#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/utils/translation.hxx>
#include <languages/fstrips/language.hxx>

#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;


namespace fs0 {
	class RelaxedState;
}

namespace fs0 { namespace gecode {


/**
 * A CSP translator keeps track of the correspondence between Planning variables and CSP variables.
 * To this end, it keeps a mapping of the form <x, t> --> y, where:
 *   - 'x' is the ID of a planning variable
 *   - 't' is the type or role under which this variable can appear on a CSP
 *     (e.g. the same state variable 'val(c1)' could appear in a certain CSP as input _and_ output variable of an effect
 *   - 'y' is the identifier of the CSP variable (i.e. implementation-wise, the position in which the variable appears on the '_X' IntVarArray of the CSP)
 */
class GecodeCSPVariableTranslator {
public:

	
	GecodeCSPVariableTranslator() {};
	virtual ~GecodeCSPVariableTranslator() {}

	//! Forbid copy constructor
	explicit GecodeCSPVariableTranslator(GecodeCSPVariableTranslator& other);
	
	
	//! Returns true iff the given term has already an associated CSP variable
	bool isRegistered(const fs::Term::cptr term, CSPVariableType type) const;

	//! Register the given term (under the give role/type) by creating a corresponding CSP variable.
	//! Returns true iff the (variable, type) tuple was actually registered for the first time (i.e. had not been registered yet)	
	bool registerConstant(fs::Constant::cptr constant, SimpleCSP& csp, Gecode::IntVarArgs& variables);
	bool registerStateVariable(fs::StateVariable::cptr variable, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& variables);
	bool registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& variables);
	bool registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, TypeIdx domain_type, SimpleCSP& csp, Gecode::IntVarArgs& variables);
	bool registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, int min, int max, SimpleCSP& csp, Gecode::IntVarArgs& variables);
	
	//! Returns the Gecode CSP variable that corresponds to the given term under the given role, for the given CSP
	const Gecode::IntVar& resolveVariable(fs::Term::cptr term, CSPVariableType type, const SimpleCSP& csp) const;
	
	//! Handy helper to resolve a number of variables at the same time
	Gecode::IntVarArgs resolveVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, const SimpleCSP& csp) const;
	
	
	//! The key operation in the RPG progression: to update the domains of the relevant state variables for a certain layer of the RPG.
	void updateStateVariableDomains(SimpleCSP& csp, const RelaxedState& layer) const;

	//! Returns the set of Gecode CSP variables that corresponds to all the state variables derived from a function
	//! when used according to the given type.
	Gecode::IntVarArgs resolveFunction(const SimpleCSP& csp, unsigned symbol_id, CSPVariableType type) const;
	
	
	//! Returns the value of the CSP variable that corresponds to the given INPUT state variable, in the given CSP.
	const ObjectIdx resolveInputStateVariableValue(const SimpleCSP& csp, VariableIdx variable) const {
		const auto& it = _input_state_variables.find(variable);
		if (it == _input_state_variables.end()) throw std::runtime_error("Trying to resolve non-registered input state variable");
		const Gecode::IntVar& csp_var = csp._X[it->second];
		return csp_var.val();
	}
	
	//! Returns the value of the CSP variable that corresponds to the given INPUT state variable, in the given CSP.
	const ObjectIdx resolveOutputStateVariableValue(const SimpleCSP& csp, VariableIdx variable) const {
		const auto& it = _output_state_variables.find(variable);
		if (it == _output_state_variables.end()) throw std::runtime_error("Trying to resolve non-registered output state variable");
		const Gecode::IntVar& csp_var = csp._X[it->second];
		return csp_var.val();
	}	
	
	const std::unordered_map<VariableIdx, unsigned>& getAllInputVariables() const { return _input_state_variables; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const SimpleCSP& csp) const;

protected:
	//! A map mapping terms that have already been processed (under a certain role, e.g. input/output) to the ID of their corresponding CSP variable
	std::unordered_map<TranslationKey, unsigned> _registered;
	
	//! Some data structures to keep track of all registered state variables, so that we can update their domains and parse their values efficiently.
	//! In particular, these maps the index of state variables that have been registered under different input/output roles to 
	//! the ID of the corresponding CSP variable
	std::unordered_map<VariableIdx, unsigned> _input_state_variables;
	std::unordered_map<VariableIdx, unsigned> _output_state_variables;
};



} } // namespaces


