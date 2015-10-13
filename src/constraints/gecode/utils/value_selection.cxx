

#include <constraints/gecode/utils/value_selection.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/base.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>

namespace fs0 { namespace gecode {

EarliestLayerValueSelector::EarliestLayerValueSelector()
	: _translator(nullptr), _bookkeeping(nullptr)
{}

EarliestLayerValueSelector::EarliestLayerValueSelector(const GecodeCSPVariableTranslator* translator, const RPGData* bookkeeping)
	: _translator(translator), _bookkeeping(bookkeeping)
{}

int EarliestLayerValueSelector::select(const Gecode::IntVar& x, unsigned csp_var_idx) const {
	assert(_translator && _bookkeeping);
	
	VariableIdx variable = _translator->getPlanningVariable(csp_var_idx);
	
	// If the CSP variable does NOT model a planning state variable, we simply return the minimum available value for maximum performance
	if (variable == INVALID_VARIABLE) return x.min();
	
	int best_value = std::numeric_limits<int>::max();
	unsigned smallest_layer = std::numeric_limits<unsigned>::max();
	
	Gecode::IntVarValues values(x);
	assert(values()); // We require at least one value, so that 'best_value' gets set at least once.
	
	
	for (; values(); ++values) {
		int value = values.val();
		if (value == DONT_CARE::get()) continue; // Don't ever select dont_care values
		
		const auto& support = _bookkeeping->getAtomSupport(Atom(variable, value));
		unsigned layer = std::get<0>(support); // The RPG layer on which this value was first achieved for this variable

		if (layer == 0) return value; // If we found a seed-state value, no need to search anymore
		if (layer < smallest_layer) {
			smallest_layer = layer;
			best_value = value;
		}
	}
	return best_value;
}


} } // namespaces
