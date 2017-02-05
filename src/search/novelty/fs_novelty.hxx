
#pragma once

#include <memory>
#include <cassert>

#include <lapkt/novelty/multivalued_evaluator.hxx>
#include <lapkt/novelty/atom_evaluator.hxx>
#include <utils/atom_index.hxx>


namespace fs0 { namespace bfws {


using FSBinaryNoveltyEvaluatorI = lapkt::novelty::NoveltyEvaluatorI<bool>;
using FSBinaryNoveltyEvaluator = lapkt::novelty::MultivaluedNoveltyEvaluator<bool>;

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

using FSAtomNoveltyEvaluator = lapkt::novelty::AtomNoveltyEvaluator<bool, FSAtomValuationIndexer, lapkt::novelty::BoolVectorTuple2Marker>;

/*
class FSAtomNoveltyEvaluator : public lapkt::novelty::AtomNoveltyEvaluator<bool, FSAtomValuationIndexer> {
public:
	using Base = AtomNoveltyEvaluator<bool, FSAtomValuationIndexer>;

	IWBinaryNoveltyEvaluator(unsigned novelty_bound) :
		Base(novelty_bound)
	{}
	
	~IWBinaryNoveltyEvaluator() = default;
	IWBinaryNoveltyEvaluator(const IWBinaryNoveltyEvaluator&) = default;
	
	using Base::evaluate; // So that we do not hide the base evaluate(const FiniteDomainNoveltyEvaluator&) method
	
	unsigned _evaluate(const lapkt::novelty::BinaryFeatureValuation& valuation, unsigned check_only_width = 0) {
		std::size_t num_features = valuation.size();
		if (_all_features_novel.size() != num_features) {
			_all_features_novel.resize(num_features);
			std::iota(_all_features_novel.begin(), _all_features_novel.end(), 0);
		}
		
		return check_only_width > 0 ? evaluate(valuation, _all_features_novel, check_only_width) : evaluate(valuation, _all_features_novel);
	}
	
	//! Evaluate the novelty of a node taking into account the valuation of its parent, for optimization purposes
	unsigned _evaluate(const lapkt::novelty::BinaryFeatureValuation& valuation, const lapkt::novelty::BinaryFeatureValuation& parent_valuation, unsigned check_only_width = 0) {
		std::vector<unsigned> novel = derive_novel(valuation, parent_valuation);
		return check_only_width > 0 ? evaluate(valuation, novel, check_only_width) : evaluate(valuation, novel);
	}
	
protected:
	//! This is used to cache a vector <0,1,...,k> of appropriate length and spare the creation of one each time we need it.
	mutable std::vector<unsigned> _all_features_novel;
};

*/

} } // namespaces
