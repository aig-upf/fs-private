
#pragma once

#include <gecode/int.hh>

namespace fs0 { class RPGData; }

namespace fs0 { namespace gecode {

class GecodeCSPVariableTranslator;

//! A value selector for Gecode variables that favors those values of the given variable
//! that were achieved earliest in the relaxed planning graph
class EarliestLayerValueSelector {
public:
	//! Parameter-less constructor, creates a selector in an invalid state
	EarliestLayerValueSelector();
	
	//! The proper constructor
	EarliestLayerValueSelector(const GecodeCSPVariableTranslator* translator, const RPGData* bookkeeping);

	EarliestLayerValueSelector(const EarliestLayerValueSelector&) = default;
	EarliestLayerValueSelector(EarliestLayerValueSelector&&) = default;
	EarliestLayerValueSelector& operator=(const EarliestLayerValueSelector&) = default;
	EarliestLayerValueSelector& operator=(EarliestLayerValueSelector&&) = default;
	
	int select(const Gecode::IntVar& x, unsigned csp_var_idx) const;

protected:
	//! Pointers to the necessary data structures to perform the value selection
	const GecodeCSPVariableTranslator* _translator;
	const RPGData* _bookkeeping;
};


} } // namespaces

