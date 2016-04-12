
#pragma once

#include <unordered_map>
#include <unordered_set>

#include <fs_types.hxx>
#include <constraints/gecode/utils/translation.hxx>

#include <gecode/int.hh>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips { class Constant; class NestedTerm; class BoundVariable; class Term; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class SimpleCSP;
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
	GecodeCSPVariableTranslator(const GecodeCSPVariableTranslator&) = delete;
	GecodeCSPVariableTranslator(GecodeCSPVariableTranslator&&) = delete;
	GecodeCSPVariableTranslator& operator=(const GecodeCSPVariableTranslator& rhs) = delete;
	GecodeCSPVariableTranslator& operator=(GecodeCSPVariableTranslator&& rhs) = delete;

	unsigned add_intvar(Gecode::IntVar csp_variable, VariableIdx planning_variable = INVALID_VARIABLE);
	unsigned add_boolvar(Gecode::BoolVar csp_variable);

	//! This updates the actual variables of the given CSP with all the CSP variables that have been registered in the translator so far
	void perform_registration();

	//! Register the given term (under the give role/type) by creating a corresponding CSP variable.
	//! Returns true iff the (variable, type) tuple was actually registered for the first time (i.e. had not been registered yet)
	bool registerConstant(const fs::Constant* constant);
	
	void registerExistentialVariable(const fs::BoundVariable* variable);
	
	//! Register an input variable, i.e. a CSP variable directly related to a planning state variable.
	void registerInputStateVariable(VariableIdx variable);
	
	//! Register an int variable not boud to any particular term, and return its index.
	unsigned registerIntVariable(int min, int max);
	
	bool registerNestedTerm(const fs::NestedTerm* nested, CSPVariableType type);
	bool registerNestedTerm(const fs::NestedTerm* nested, CSPVariableType type, TypeIdx domain_type);
	bool registerNestedTerm(const fs::NestedTerm* nested, CSPVariableType type, int min, int max);
	
	//! Returns the index of the CSP variable corresponding to the given term under the given role.
	unsigned resolveVariableIndex(const fs::Term* term, CSPVariableType type) const;
	
	//! Returns the Gecode CSP variable that corresponds to the given term under the given role, for the given CSP
	const Gecode::IntVar& resolveVariable(const fs::Term* term, CSPVariableType type, const SimpleCSP& csp) const;
	
	//! Returns the value of the Gecode CSP variable that corresponds to the given term under the given role, for the given CSP
	ObjectIdx resolveValue(const fs::Term* term, CSPVariableType type, const SimpleCSP& csp) const;
	
	const Gecode::IntVar& resolveVariableFromIndex(unsigned variable_index, const SimpleCSP& csp) const;
	ObjectIdx resolveValueFromIndex(unsigned variable_index, const SimpleCSP& csp) const;

	//! Helper to resolve several variables at the same time
	Gecode::IntVarArgs resolveVariables(const std::vector<const fs::Term*>& terms, CSPVariableType type, const SimpleCSP& csp) const;
	std::vector<ObjectIdx> resolveValues(const std::vector<const fs::Term*>& terms, CSPVariableType type, const SimpleCSP& csp) const;

	//! The key operation in the RPG progression: to update the domains of the relevant state variables for a certain layer of the RPG.
	void updateStateVariableDomains(SimpleCSP& csp, const GecodeRPGLayer& layer) const;
	void updateStateVariableDomains(SimpleCSP& csp, const std::vector<Gecode::IntSet>& domains) const;
	void updateStateVariableDomains(SimpleCSP& csp, const State& state) const;

	const unsigned resolveInputVariableIndex(VariableIdx variable) const {
		const auto& it = _input_state_variables.find(variable);
		if (it == _input_state_variables.end()) throw UnregisteredStateVariableError("Trying to resolve non-registered input state variable");
		return it->second;
	}
	
	//! Returns the CSP variable that corresponds to the given input state variable, in the given CSP.
	const Gecode::IntVar& resolveInputStateVariable(const SimpleCSP& csp, VariableIdx variable) const;

	//! Returns the value of the CSP variable that corresponds to the given input state variable, in the given CSP.
	const ObjectIdx resolveInputStateVariableValue(const SimpleCSP& csp, VariableIdx variable) const {
		return resolveInputStateVariable(csp, variable).val();
	}

	//! Creates a new boolean CSP variable and returns its index
	unsigned create_bool_variable();

	const std::unordered_map<VariableIdx, unsigned>& getAllInputVariables() const { return _input_state_variables; }
	
	
	//! Returns a partial assignment of values to the input state variables of the CSP managed by this translator, built from the given solution.
	PartialAssignment buildAssignment(SimpleCSP& solution) const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeCSPVariableTranslator& o) { return o.print(os, o._base_csp); }
	std::ostream& print(std::ostream& os, const SimpleCSP& csp) const;
	
	SimpleCSP& getBaseCSP() { return _base_csp; }
	
	VariableIdx getPlanningVariable(unsigned csp_var_idx) const;
	
	std::vector<std::pair<unsigned, std::vector<unsigned>>> index_fluents(const std::unordered_set<const fs::Term*>& terms);
	
protected:
	//! The base CSP object upon which static variable and constraint registration processes act.
	SimpleCSP& _base_csp;
	
	// The list of integer and boolean CSP variables that is created during the variable registration state
	Gecode::IntVarArgs _intvars;
	Gecode::BoolVarArgs _boolvars;
	
	//! An index - _intvars_idx[x] is the VariableIdx of the CSP variable with index 'x'
	std::vector<VariableIdx> _intvars_idx;
	
	//! A map mapping terms that have already been processed (under a certain role, e.g. input/output) to the ID of their corresponding CSP variable
	std::unordered_map<TranslationKey, unsigned> _registered;
	
	//! Some data structures to keep track of all registered state variables, so that we can update their domains and parse their values efficiently.
	//! In particular, these map the index of state variables that have been registered under different input/output roles to
	//! the ID of the corresponding CSP variable
	std::unordered_map<VariableIdx, unsigned> _input_state_variables;
// 	std::unordered_map<VariableIdx, unsigned> _output_state_variables;
};



} } // namespaces
