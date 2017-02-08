
#pragma once

#include <memory>
#include <cassert>

#include <lapkt/novelty/multivalued_evaluator.hxx>
#include <lapkt/novelty/atom_evaluator.hxx>
#include <utils/atom_index.hxx>


namespace fs0 { namespace bfws {

//! An auxiliary class to inject into novelty evaluators in order to convert 
//! atoms into atom indexes. Good to keep the evaluators agnostic wrt the data
//! structures needed to perform this indexing operation
//! Works only for "state-variable" novelty features, i.e. features that exactly
//! represent the value of some state variable
class FSAtomValuationIndexer {
public:
	FSAtomValuationIndexer(const AtomIndex& atom_index) : 
		_atom_index(atom_index)
	{}
	
	FSAtomValuationIndexer(const FSAtomValuationIndexer&) = default;
	
	inline unsigned num_indexes() const {
		return _atom_index.size();
	}
	
	inline unsigned to_index(unsigned variable, int value) const {
		return _atom_index.to_index(variable, value);
	}
	
protected:
	const AtomIndex& _atom_index;
};



//! Interfaces for both binary and multivalued novelty evaluators
using FSBinaryNoveltyEvaluatorI = lapkt::novelty::NoveltyEvaluatorI<bool>;
using FSMultivaluedNoveltyEvaluatorI = lapkt::novelty::NoveltyEvaluatorI<int>;

//! A generic evaluator for sets of binary novelty features
using FSGenericBinaryNoveltyEvaluator = lapkt::novelty::GenericNoveltyEvaluator<bool>;

//! A generic evaluator for sets of multivalued novelty features
using FSGenericMultivaluedNoveltyEvaluator = lapkt::novelty::GenericNoveltyEvaluator<int>;

//! An specialized evaluator for sets of atom-based-only novelty features which additionally have to be binary
using FSAtomBinaryNoveltyEvaluator = lapkt::novelty::AtomNoveltyEvaluator<bool, FSAtomValuationIndexer, lapkt::novelty::BoolVectorTuple2Marker>;
using FSAtomBinaryNoveltyEvaluatorPersistent = lapkt::novelty::AtomNoveltyEvaluator<bool, FSAtomValuationIndexer, lapkt::novelty::PersistentBoolVectorTuple2Marker>;



} } // namespaces
