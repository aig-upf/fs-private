
#pragma once

#include <languages/fstrips/language.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

	class NestedFluentData {
	public:
		NestedFluentData(unsigned index_position, unsigned first_boolean_position, unsigned table_size)
			: _index_position(index_position), _first_boolean_position(first_boolean_position), _table_size(table_size), _table_variables(table_size)
		{}
		
		//! Returns the Gecode temporary variable for the index of the element constraint
		const Gecode::IntVar& getIndex(const SimpleCSP& csp) const {
			return csp._intvars[_index_position];
		}
		
		//! Returns the Gecode boolean variables for the reification of the index variables
		Gecode::BoolVarArgs getIndexReificationVariables(const SimpleCSP& csp) const {
			Gecode::BoolVarArgs variables;
			for (unsigned i = 0; i < 2 * _table_size; i += 2) {
				variables << csp._boolvars[_first_boolean_position + i];
			}
			return variables;
		}
		
		//! Returns the Gecode boolean variables for the reification of the table variables
		Gecode::BoolVarArgs getTableReificationVariables(const SimpleCSP& csp) const {
			Gecode::BoolVarArgs variables;
			for (unsigned i = 0; i < 2 * _table_size; i += 2) {
				// We recover the boolean variables previously created, interleaved
				variables << csp._boolvars[_first_boolean_position + i + 1];
			}
			return variables;
		}
		
		std::vector<VariableIdx>& getTableVariables() { return _table_variables; }
		
		//! Returns the ID of the state variable into which the current nested fluent resolves under the given CSP
		VariableIdx resolveStateVariable(const SimpleCSP& csp) const {
			unsigned idx = getIndex(csp).val();
			return _table_variables[idx];
		}

	protected:
		//! The index (within the CSP _intvars array) of the element constraint index variable;
		unsigned _index_position;
		
		//! The index (within the CSP _boolvars array) of the first boolean variable of the set of variables that we use for reification purposes,
		unsigned _first_boolean_position;
		
		//! The size of the element constraint table
		unsigned _table_size;
		
		// A mapping between the implicit index of the array and the actual derived variable in the element constraint table
		std::vector<VariableIdx> _table_variables;
	};
	
} } // namespaces
