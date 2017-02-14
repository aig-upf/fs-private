

#include "atom_evaluator.hxx"


namespace lapkt { namespace novelty {

uint32_t _combine_indexes(uint32_t index1, uint32_t index2, uint32_t num_atom_indexes) {
	assert(index1 != index2);
	uint32_t small, large;
	if (index1 < index2) {
		small = index1;
		large = index2;
	} else {
		small = index2;
		large = index1;
	}
	
	return num_atom_indexes*small + large;
}

} } // namespaces
