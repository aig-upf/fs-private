
#pragma once

#include <unordered_map>

#include <gecode/int.hh>

#include <fs_types.hxx>

namespace fs0 { class AtomIndex; }

namespace fs0 { namespace gecode {

class CSPTranslator;
class RPGIndex;

//! A value selector for Gecode variables that favors those values of the given variable
//! that were achieved earliest in the relaxed planning graph.
//! By default, however (when the selector is constructed with no parameters), the selected value
//! is the minimum one.
class MinHMaxValueSelector {
public:
	virtual ~MinHMaxValueSelector() {}
	
	//! The actual value selection policy method
	virtual int select(const Gecode::IntVar& x, unsigned csp_var_idx) const = 0;
};


//! A tuple-based version 
class TupleMinHMaxValueSelector : public MinHMaxValueSelector {
public:
	//! The proper constructor
	TupleMinHMaxValueSelector(const AtomIndex* tuple_index, const CSPTranslator* translator, const RPGIndex* bookkeeping);

	TupleMinHMaxValueSelector(const TupleMinHMaxValueSelector&) = default;
	TupleMinHMaxValueSelector(TupleMinHMaxValueSelector&&) = default;
	TupleMinHMaxValueSelector& operator=(const TupleMinHMaxValueSelector&) = default;
	TupleMinHMaxValueSelector& operator=(TupleMinHMaxValueSelector&&) = default;
	
	//! The actual value selection policy method
	int select(const Gecode::IntVar& x, unsigned csp_var_idx) const;

protected:
	//! Pointers to the necessary data structures to perform the value selection
	const AtomIndex* _tuple_index; 
	const CSPTranslator* _translator;
	const RPGIndex* _bookkeeping;
	
	int select_state_variable_value(VariableIdx variable, const Gecode::IntVar& x) const;
	
	int select_existential_variable_value(const std::vector<std::unordered_map<int, AtomIdx>>& existential_data, const Gecode::IntVar& x) const;
};

} } // namespaces

