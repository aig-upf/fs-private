
#pragma once

#include <gecode/int.hh>

namespace fs0 { class RPGData; }

namespace fs0 { namespace gecode {

class GecodeCSPVariableTranslator;

//! A value selector for Gecode variables that favors those values of the given variable
//! that were achieved earliest in the relaxed planning graph.
//! By default, however (when the selector is constructed with no parameters), the selected value
//! is the minimum one.
class MinHMaxValueSelector {
public:
	//! Parameter-less constructor, creates a selector in an "default" state that simply applies a min-value selection policy
	MinHMaxValueSelector();
	
	//! The proper constructor
	MinHMaxValueSelector(const GecodeCSPVariableTranslator* translator, const RPGData* bookkeeping);

	MinHMaxValueSelector(const MinHMaxValueSelector&) = default;
	MinHMaxValueSelector(MinHMaxValueSelector&&) = default;
	MinHMaxValueSelector& operator=(const MinHMaxValueSelector&) = default;
	MinHMaxValueSelector& operator=(MinHMaxValueSelector&&) = default;
	
	//! The actual value selection policy method
	int select(const Gecode::IntVar& x, unsigned csp_var_idx) const;

protected:
	//! Pointers to the necessary data structures to perform the value selection
	const GecodeCSPVariableTranslator* _translator;
	const RPGData* _bookkeeping;
};


} } // namespaces

