

#include "fs_novelty.hxx"
#include <utils/atom_index.hxx>

namespace fs0 { namespace bfws {

unsigned FSAtomValuationIndexer::num_indexes() const {
	return _atom_index.size();
}

unsigned FSAtomValuationIndexer::to_index(unsigned variable, int value) const {
	return _atom_index.to_index(variable, value);
}

const Atom& FSAtomValuationIndexer::to_atom(unsigned index) const {
	return _atom_index.to_atom(index);
}

} } // namespaces
