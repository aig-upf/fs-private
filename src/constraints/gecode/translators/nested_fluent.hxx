
#pragma once

#include <languages/fstrips/language.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

	class NestedFluentData {
	public:
		NestedFluentData() {}
		
		void addTableReificationVariable(unsigned csp_variable_index) {
			_table_reification_variables.push_back(csp_variable_index);
		}
		
		void addIndexReificationVariable(unsigned csp_variable_index) {
			_index_reification_variables.push_back(csp_variable_index);
		}
		
		void setIndex(unsigned index) { _index_position = index; }
		
		//! Returns the Gecode temporary variable for the index of the element constraint
		const Gecode::IntVar& getIndex(const SimpleCSP& csp) const {
			return csp._intvars[_index_position];
		}
		
		const Gecode::BoolVar& getIndexReificationVariable(const SimpleCSP& csp, unsigned iteration_index) const {
			return csp._boolvars[_index_reification_variables[iteration_index]];
		}
		
		const Gecode::BoolVar& getTableReificationVariable(const SimpleCSP& csp, unsigned iteration_index) const {
			return csp._boolvars[_table_reification_variables[iteration_index]];
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
		
		//! The positions in the CSP boolean variables vector of the table reification variables corresponding to each index in the element table
		std::vector<unsigned> _table_reification_variables;
		
		//! The positions in the CSP boolean variables vector of the index reification variables corresponding to each index in the element table
		std::vector<unsigned> _index_reification_variables;
		
		// A mapping between the implicit index of the array and the actual derived variable in the element constraint table
		std::vector<VariableIdx> _table_variables;
	};
	
} } // namespaces
