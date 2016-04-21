

#include <constraints/gecode/utils/value_selection.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/tuple_index.hxx>

namespace fs0 { namespace gecode {


TupleMinHMaxValueSelector::TupleMinHMaxValueSelector(const TupleIndex* tuple_index, const CSPTranslator* translator, const RPGIndex* bookkeeping)
	: _tuple_index(tuple_index), _translator(translator), _bookkeeping(bookkeeping)
{}

int TupleMinHMaxValueSelector::select(const Gecode::IntVar& x, unsigned csp_var_idx) const {
	if(!_translator || !_bookkeeping) return x.min(); // If the value selector has not been initialized, we simply fall back to a min-value selection policy
	
	VariableIdx variable = _translator->getPlanningVariable(csp_var_idx);
	if (variable != INVALID_VARIABLE) {
		return select_state_variable_value(variable, x);
	}
	
	const std::vector<std::unordered_map<int, TupleIdx>>& existential_data = _translator->getExistentialData(csp_var_idx);
	if (!existential_data.empty()) {
		return select_existential_variable_value(existential_data, x);
	}
	
	// If the CSP variable does NOT model a planning state variable, we simply return the minimum available value for maximum performance
	return x.min();
}

int TupleMinHMaxValueSelector::select_state_variable_value(VariableIdx variable, const Gecode::IntVar& x) const {
	int best_value = std::numeric_limits<int>::max();
	unsigned smallest_layer = std::numeric_limits<unsigned>::max();
	
	Gecode::IntVarValues values(x);
	assert(values()); // We require at least one value, so that 'best_value' gets set at least once.
	
	for (; values(); ++values) {
		int value = values.val();
		const auto& support = _bookkeeping->getTupleSupport(_tuple_index->to_index(variable, value));
		unsigned layer = std::get<0>(support); // The RPG layer on which this value was first achieved for this variable

		if (layer == 0) return value; // If we found a seed-state value, no need to search anymore
		if (layer < smallest_layer) {
			smallest_layer = layer;
			best_value = value;
		}
	}
	
	return best_value;
}


int TupleMinHMaxValueSelector::select_existential_variable_value(const std::vector<std::unordered_map<int, TupleIdx>>& existential_data, const Gecode::IntVar& x) const {
	int best_value = std::numeric_limits<int>::max();
	unsigned best_hmax_sum = std::numeric_limits<unsigned>::max();
	
	Gecode::IntVarValues values(x);
	assert(values()); // We require at least one value, so that 'best_value' gets set at least once.
	
	for (; values(); ++values) {
		int value = values.val();
		
		unsigned hmax_sum = 0;
		for (const std::unordered_map<int, TupleIdx>& map:existential_data) {
			const auto& it = map.find(value);
			assert(it != map.end());
			
			TupleIdx tuple = it->second;
			if (!_bookkeeping->reached(tuple)) {
				hmax_sum = std::numeric_limits<unsigned>::max();
			}
			
			hmax_sum += std::get<0>(_bookkeeping->getTupleSupport(tuple)); // The RPG layer on which this value was first achieved for this variable
		}
		
		if (hmax_sum < best_hmax_sum) {
			best_hmax_sum = hmax_sum;
			best_value = value;
		}
	}
	
// 	std::cout << "Selected value: " << best_value << std::endl;
	
	return best_value < std::numeric_limits<int>::max() ? best_value : x.min();
}

} } // namespaces
