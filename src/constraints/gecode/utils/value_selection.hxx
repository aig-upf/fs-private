
#pragma once

#include <gecode/int.hh>

namespace fs0 { class RPGData; class TupleIndex; }

namespace fs0 { namespace gecode {

class GecodeCSPVariableTranslator;
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


class AtomMinHMaxValueSelector : public MinHMaxValueSelector {
public:
	//! The proper constructor
	AtomMinHMaxValueSelector(const GecodeCSPVariableTranslator* translator, const RPGData* bookkeeping);

	AtomMinHMaxValueSelector(const AtomMinHMaxValueSelector&) = default;
	AtomMinHMaxValueSelector(AtomMinHMaxValueSelector&&) = default;
	AtomMinHMaxValueSelector& operator=(const AtomMinHMaxValueSelector&) = default;
	AtomMinHMaxValueSelector& operator=(AtomMinHMaxValueSelector&&) = default;
	
	//! The actual value selection policy method
	int select(const Gecode::IntVar& x, unsigned csp_var_idx) const;

protected:
	//! Pointers to the necessary data structures to perform the value selection
	const GecodeCSPVariableTranslator* _translator;
	const RPGData* _bookkeeping;
};

//! A tuple-based version 
class TupleMinHMaxValueSelector : public MinHMaxValueSelector {
public:
	//! The proper constructor
	TupleMinHMaxValueSelector(const TupleIndex* tuple_index, const GecodeCSPVariableTranslator* translator, const RPGIndex* bookkeeping);

	TupleMinHMaxValueSelector(const TupleMinHMaxValueSelector&) = default;
	TupleMinHMaxValueSelector(TupleMinHMaxValueSelector&&) = default;
	TupleMinHMaxValueSelector& operator=(const TupleMinHMaxValueSelector&) = default;
	TupleMinHMaxValueSelector& operator=(TupleMinHMaxValueSelector&&) = default;
	
	//! The actual value selection policy method
	int select(const Gecode::IntVar& x, unsigned csp_var_idx) const;

protected:
	//! Pointers to the necessary data structures to perform the value selection
	const TupleIndex* _tuple_index; 
	const GecodeCSPVariableTranslator* _translator;
	const RPGIndex* _bookkeeping;
};

} } // namespaces

