
#pragma once

#include <gecode/int.hh>
#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <cassert>
#include <vector>
#include <actions.hxx>
#include <atoms.hxx>
#include "simple_csp.hxx"

namespace fs0 { namespace gecode {

/**
 * A CSP translator keeps track of the correspondence between Planning variables and CSP variables.
 * To this end, it keeps a mapping of the form <x, t> --> y, where:
 *   - 'x' is the ID of a planning variable
 *   - 't' is the type or role under which this variable can appear on a CSP
 *     (e.g. the same state variable 'val(c1)' could appear in a certain CSP as input _and_ output variable of an effect
 *   - 'y' is the identifier of the CSP variable (i.e. implementation-wise, the position in which the variable appears on the '_X' IntVarArray of the CSP)
 */
class GecodeCSPTranslator {
public:
	//! The possible types of CSP variables that we might consider
	enum class VariableType {
		Input, // a variable relevant to some formula (in an action precondition, goal, etc.)
		Output // a variable affected by some action effect
	};
	
	GecodeCSPTranslator() {};
	virtual ~GecodeCSPTranslator() {}

	//! Forbid copy constructor
	explicit GecodeCSPTranslator(GecodeCSPTranslator& other);
	
	void registerCSPVariable(VariableIdx variable, VariableType type, unsigned csp_variable) {
		auto res = _variables.insert(std::make_pair(
			std::make_pair(variable, type),
			csp_variable
		));
		if (!res.second) {
			throw std::runtime_error("Tried to register a CSP variable that already existed: <" + std::to_string(variable) + ">");
		}
	}
	
	//! Returns the Gecode CSP variable that corresponds to the given state variable and type, in the given CSP.
	const Gecode::IntVar& resolveVariable(const SimpleCSP& csp, VariableIdx variable, VariableType type) const {
		auto it = _variables.find(std::make_pair(variable, type));
		if(it == _variables.end()) {
			throw std::runtime_error("Trying to translate a non-existing CSP variable");
		}
		return csp._X[ it->second ];
	}
	
	//! Returns the value of the CSP variable that corresponds to the given state variable and type, in the given CSP.
	inline const ObjectIdx resolveValue(const SimpleCSP& csp, VariableIdx variable, VariableType type) const {
		return resolveVariable(csp, variable, type).val();
	}


	//! Prints a representation of a CSP. Mostly for debugging purposes
    // friend std::ostream& operator<<(std::ostream &os, const GecodeCSPTranslator&  csp) { return csp.print(os); }
    // std::ostream& print(std::ostream& os) const { os << _X; return os; }

public:
	typedef std::pair<VariableIdx, VariableType> CSPVariableIdentifier;
protected:
	//! Variable mapping: For a state variable X, variables[X] is the (implicit, unsigned) ID of corresponding Gecode CSP variable.
	std::map<CSPVariableIdentifier, unsigned> _variables;
};


} } // namespaces
