
#pragma once

#include <unordered_map>
#include <unordered_set>

#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/utils/translation.hxx>
#include <constraints/gecode/translators/nested_fluent.hxx>
#include <languages/fstrips/language.hxx>

#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;


namespace fs0 { namespace gecode {

class GecodeRPGLayer;

/**
 * This exception is thrown whenever a variable cannot be resolved
 */
class UnregisteredStateVariableError : public std::runtime_error {
public:
	UnregisteredStateVariableError( const char* what_msg ) : std::runtime_error( what_msg ) {}
	UnregisteredStateVariableError( const std::string& what_msg ) : std::runtime_error( what_msg ) {}
};

/**
 * A CSP translator keeps track of the correspondence between Planning variables and CSP variables.
 * To this end, it keeps a mapping of the form <x, t> --> y, where:
 *   - 'x' is the ID of a planning variable
 *   - 't' is the type or role under which this variable can appear on a CSP
 *     (e.g. the same state variable 'val(c1)' could appear in a certain CSP as input _and_ output variable of an effect
 *   - 'y' is the identifier of the CSP variable (i.e. implementation-wise, the position in which the variable appears on the '_intvars' IntVarArray of the CSP)
 */
class GecodeCSPVariableTranslator {
public:

	GecodeCSPVariableTranslator(SimpleCSP& base_csp) : _base_csp(base_csp) {};
	virtual ~GecodeCSPVariableTranslator() {}

	//! Forbid copy constructor
	explicit GecodeCSPVariableTranslator(GecodeCSPVariableTranslator& other);


	//! Returns true iff the given term has already an associated CSP variable
	bool isRegistered(const fs::Term::cptr term, CSPVariableType type) const;
	
	//! Returns true iff the constraints that correspond to the term / type have already been posted
	bool isPosted(const fs::Term::cptr term, CSPVariableType type) const;
	
	//! Marks the given pair of term / type as with its corresponding constraint having been posted
	void setPosted(const fs::Term::cptr term, CSPVariableType type);
	
	//! This updates the actual variables of the given CSP with all the CSP variables that have been registered in the translator so far
	void perform_registration();

	//! Register the given term (under the give role/type) by creating a corresponding CSP variable.
	//! Returns true iff the (variable, type) tuple was actually registered for the first time (i.e. had not been registered yet)
	bool registerConstant(fs::Constant::cptr constant);
	
	bool registerStateVariable(fs::StateVariable::cptr variable, CSPVariableType type);
	
	//! Register a state variable which is derived from a nested fluent
	bool registerDerivedStateVariable(VariableIdx variable, CSPVariableType type);
	
	bool registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type);
	bool registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, TypeIdx domain_type);
	bool registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, int min, int max);

	//! Returns the index of the CSP variable corresponding to the given term under the given role.
	unsigned resolveVariableIndex(fs::Term::cptr term, CSPVariableType type) const;
	
	//! Returns the Gecode CSP variable that corresponds to the given term under the given role, for the given CSP
	const Gecode::IntVar& resolveVariable(fs::Term::cptr term, CSPVariableType type, const SimpleCSP& csp) const;
	
	//! Returns the value of the Gecode CSP variable that corresponds to the given term under the given role, for the given CSP
	ObjectIdx resolveValue(fs::Term::cptr term, CSPVariableType type, const SimpleCSP& csp) const;
	
	ObjectIdx resolveValueFromIndex(unsigned variable_index, const SimpleCSP& csp) const;

	//! Handy helper to resolve a number of variables at the same time
	Gecode::IntVarArgs resolveVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, const SimpleCSP& csp) const;

	//! Registers auxiliary data necessary for the element constraint that corresponds to the given term
	void registerNestedFluent(fs::NestedTerm::cptr term, unsigned table_size);
	
	//! Returns auxiliary data necessary for the element constraint that corresponds to the given term. Const and non-const versions
	NestedFluentData& resolveNestedFluent(fs::Term::cptr term);
	const NestedFluentData& resolveNestedFluent(fs::Term::cptr term) const;


	//! The key operation in the RPG progression: to update the domains of the relevant state variables for a certain layer of the RPG.
	void updateStateVariableDomains(SimpleCSP& csp, const GecodeRPGLayer& layer) const;

	const unsigned resolveInputVariableIndex(VariableIdx variable) const {
		const auto& it = _input_state_variables.find(variable);
		if (it == _input_state_variables.end()) throw UnregisteredStateVariableError("Trying to resolve non-registered input state variable");
		return it->second;
	}
	
	//! Returns the CSP variable that corresponds to the given input state variable, in the given CSP.
	const Gecode::IntVar& resolveInputStateVariable(const SimpleCSP& csp, VariableIdx variable) const {
		return csp._intvars[resolveInputVariableIndex(variable)];
	}

	//! Returns the value of the CSP variable that corresponds to the given input state variable, in the given CSP.
	const ObjectIdx resolveInputStateVariableValue(const SimpleCSP& csp, VariableIdx variable) const {
		return resolveInputStateVariable(csp, variable).val();
	}

	//! Returns the CSP variable that corresponds to the given output state variable, in the given CSP.
	const Gecode::IntVar& resolveOutputStateVariable(const SimpleCSP& csp, VariableIdx variable) const {
		const auto& it = _output_state_variables.find(variable);
		if (it == _output_state_variables.end()) throw UnregisteredStateVariableError("Trying to resolve non-registered output state variable");
		return csp._intvars[it->second];
	}

	//! Returns the value of the CSP variable that corresponds to the given output state variable, in the given CSP.
	const ObjectIdx resolveOutputStateVariableValue(const SimpleCSP& csp, VariableIdx variable) const {
		return resolveOutputStateVariable(csp, variable).val();
	}
	
	const unsigned resolveDerivedVariableIndex(VariableIdx variable) const {
		const auto& it = _derived.find(variable);
		if (it == _derived.end()) throw UnregisteredStateVariableError("Trying to resolve non-registered derived state variable");
		return it->second;
	}
	
	//! Returns the CSP variable that corresponds to the given derived state variable, in the given CSP.
	const Gecode::IntVar& resolveDerivedStateVariable(const SimpleCSP& csp, VariableIdx variable) const {
		return csp._intvars[resolveDerivedVariableIndex(variable)];
	}
	
	//! Creates a new boolean CSP variable and returns its index
	unsigned create_bool_variable();
	

	const std::unordered_map<VariableIdx, unsigned>& getAllInputVariables() const { return _input_state_variables; }
	
	//! Returns a partial assignment of values to the input state variables of the CSP managed by this translator, built from the given solution.
	PartialAssignment buildAssignment(SimpleCSP& solution) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const SimpleCSP& csp) const;
	
	SimpleCSP& getBaseCSP() { return _base_csp; }

protected:
	//! The base CSP object upon which static variable and constraint registration processes act.
	SimpleCSP& _base_csp;
	
	// The list of integer and boolean CSP variables that is created during the variable registration state
	Gecode::IntVarArgs _intvars;
	Gecode::BoolVarArgs _boolvars;
	
	
	//! A map mapping terms that have already been processed (under a certain role, e.g. input/output) to the ID of their corresponding CSP variable
	std::unordered_map<TranslationKey, unsigned> _registered;
	
	//! A map from terms representing derived state variables to the ID of their corresponding CSP variable
	std::unordered_map<VariableIdx, unsigned> _derived;
	
	//! A set marking all terms whose corresponding constraints have already been posted.
	std::unordered_set<TranslationKey> _posted;

	//! Some data structures to keep track of all registered state variables, so that we can update their domains and parse their values efficiently.
	//! In particular, these maps the index of state variables that have been registered under different input/output roles to
	//! the ID of the corresponding CSP variable
	std::unordered_map<VariableIdx, unsigned> _input_state_variables;
	std::unordered_map<VariableIdx, unsigned> _output_state_variables;

	//! A table keeping track of auxiliary data for the element constraint that is necessary for the given term / type.
	std::unordered_map<TranslationKey, NestedFluentData> element_constraint_data;
};



} } // namespaces
