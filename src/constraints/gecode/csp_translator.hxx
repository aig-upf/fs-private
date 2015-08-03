
#pragma once

#include <gecode/int.hh>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <cassert>
#include <vector>
#include <atoms.hxx>
#include "simple_csp.hxx"

namespace fs0 { namespace gecode {

//! The possible types of CSP variables that we might consider
enum class CSPVariableType {
	Input, // a variable relevant to some formula (in an action precondition, goal, etc.)
	Output, // a variable affected by some action effect
	Temporary,  // a temporal variable useful only for the CSP, but not coming from the planning problem
	TemporaryOutput, // a temporal variable which substitutes for the LHS term of an assignment
};
	
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
	
	//! Register the given planning variable under the give role/type as corresponding to the CSP variable with the given index.
	//! Returns true iff the (variable, type) tuple was actually registered for the first time (i.e. had not been registered yet)
	bool registerCSPVariable(VariableIdx variable, CSPVariableType type, unsigned csp_variable);
	
	//! Returns the Gecode CSP variable that corresponds to the given state variable and type, in the given CSP.
	const Gecode::IntVar& resolveVariable(const SimpleCSP& csp, VariableIdx variable, CSPVariableType type) const;
	
	//! Returns the set of Gecode CSP variables that corresponds to all the state variables derived from a function
	//! when used according to the given type.
	Gecode::IntVarArgs resolveFunction(const SimpleCSP& csp, unsigned symbol_id, CSPVariableType type) const;
	
	//! A small helper to resolve a whole scope.
	Gecode::IntVarArgs resolveScope(const SimpleCSP& csp, const VariableIdxVector& scope, CSPVariableType type) const;
	
	//! Returns the value of the CSP variable that corresponds to the given state variable and type, in the given CSP.
	inline const ObjectIdx resolveValue(const SimpleCSP& csp, VariableIdx variable, CSPVariableType type) const {
		const Gecode::IntVar& csp_var = resolveVariable(csp, variable, type);
		return csp_var.val();
	}

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const SimpleCSP& csp) const;

protected:
	typedef std::pair<VariableIdx, CSPVariableType> CSPVariableIdentifier;
	
	//! Variable mapping: For a state variable X, variables[X] is the (implicit, unsigned) ID of corresponding Gecode CSP variable.
	std::map<CSPVariableIdentifier, unsigned> _variables;
};


} } // namespaces
