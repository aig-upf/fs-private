/*
 * This file contains some classes necessary to handle nested fluent terms 
 * by mapping them into element constraints.
 * 
 */
#pragma once

#include <vector>
#include <ostream> 

#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/gecode_csp.hxx>
#include <gecode/int.hh>


namespace fs = fs0::language::fstrips;


namespace fs0 { namespace gecode {


//! A compact class that indexes the number of occurrences of state variables in our CSPs,
//! making a distinction between direct and derived state variables, and encapsulating the logic
//! that allows us to decide when a certain (derived) state variable is nullable or not.
class VariableCounter {
public:
	VariableCounter() = default;
	
	//! Increase the counters
	void count_flat(VariableIdx variable) { count(variable, _in_flat_term); }
	void count_nested(VariableIdx variable) { count(variable, _in_nested_term); }
	
	unsigned num_flat_occurrences(VariableIdx variable) const { return num_occurrences(variable, _in_flat_term); }
	unsigned num_nested_occurrences(VariableIdx variable) const { return num_occurrences(variable, _in_nested_term); }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const VariableCounter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		auto printer = [&os](const std::pair<VariableIdx, unsigned> count) {
			os <<  "\t\t" << ProblemInfo::getInstance().getVariableName(count.first) << ": " << count.second << std::endl;
		};
		
		os << "\tDirect State Variables: " << std::endl;
		std::for_each(_in_flat_term.cbegin(), _in_flat_term.cend(), printer);
		
		os << "\tDerived State Variables: " << std::endl;
		std::for_each(_in_nested_term.cbegin(), _in_nested_term.cend(), printer);
		
		return os;
	}
	
	bool symbol_requires_element_constraint(unsigned symbol_id) const {
		unsigned flat = num_flat_occurrences(symbol_id);
		unsigned nested = num_nested_occurrences(symbol_id);
		
		// We need to use an element constraint if there i
		return nested > 1 || (nested == 1 && flat > 0);
	}

protected:
	//! Maps from symbol ID to number of occurrences of each type
	std::map<unsigned, unsigned> _in_flat_term;
	std::map<unsigned, unsigned> _in_nested_term;
	
	static unsigned num_occurrences(VariableIdx variable, const std::map<VariableIdx, unsigned>& where) {
		auto it = where.find(variable);
		if (it == where.end()) return 0;
		return it->second;
	}
	
	static void count(VariableIdx variable, std::map<VariableIdx, unsigned>& where) {
		auto it = where.find(variable);
		if (it == where.end()) where.insert(std::make_pair(variable, 1));
		else it->second++;
	}	
};


class NestedFluentData {
public:
	NestedFluentData() = default;
	
	void setIndex(unsigned index) { _index_position = index; }
	
	//! Returns the Gecode temporary variable for the index of the element constraint
	const Gecode::IntVar& getIndex(const GecodeCSP& csp) const {
		return csp._intvars[_index_position];
	}
	
	std::vector<VariableIdx>& getTableVariables() { return _table_variables; }
	
	//! Returns the ID of the state variable into which the current nested fluent resolves under the given CSP
	VariableIdx resolveStateVariable(const GecodeCSP& csp) const {
		unsigned idx = getIndex(csp).val();
		return _table_variables[idx];
	}

protected:
	//! The index (within the CSP _intvars array) of the element constraint index variable;
	unsigned _index_position;
	
	// A mapping between the implicit index of the array and the actual derived variable in the element constraint table
	std::vector<VariableIdx> _table_variables;
};

class NestedFluentElementTranslator {
public:
	
	NestedFluentElementTranslator(const fs::FluentHeadedNestedTerm* term);
	
	void register_variables(CSPTranslator& translator);
	
	void register_constraints(CSPTranslator& translator);
	
	const fs::FluentHeadedNestedTerm* getTerm() const { return _term; }
	
	const NestedFluentData& getNestedFluentData() const { return _fluent_data; }

protected:
	
	const fs::FluentHeadedNestedTerm* _term;
	
	NestedFluentData _fluent_data;
};


} } // namespaces

