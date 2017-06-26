

#include "fs_novelty.hxx"
#include <utils/atom_index.hxx>

namespace fs0 { namespace bfws {

unsigned FSAtomValuationIndexer::num_indexes() const {
	return _atom_index.size();
}

template <typename T>
unsigned FSAtomValuationIndexer::to_index(unsigned variable, const T& value) const {
	return _atom_index.to_index(variable, make_object(value));
}

const Atom& FSAtomValuationIndexer::to_atom(unsigned index) const {
	return _atom_index.to_atom(index);
}

 // explicit instantiations
template unsigned FSAtomValuationIndexer::to_index<int>(unsigned variable, const int& value) const;
template unsigned FSAtomValuationIndexer::to_index<bool>(unsigned variable, const bool& value) const;

} } // namespaces
