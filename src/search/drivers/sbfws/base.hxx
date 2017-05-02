
#pragma once

#include "config.hxx"
#include <search/novelty/fs_novelty.hxx>

namespace fs0 { class Problem; }

namespace fs0 { namespace bfws {


//! TODO - TOO HACKY?
inline unsigned _index(unsigned unachieved, unsigned relaxed_achieved) {
	return (unachieved<<16) | relaxed_achieved;
}

// Index the novelty tables by <#g, #r>
struct SBFWSNoveltyIndexer {
	unsigned operator()(unsigned unachieved, unsigned relaxed_achieved) const {
		return _index(unachieved, relaxed_achieved);
	}

	std::tuple<unsigned, unsigned> relevant(unsigned unachieved, unsigned relaxed_achieved) const {
		return std::make_tuple(unachieved, relaxed_achieved);
	}
};

//! The NoveltyFactory decides what type of novelty evaluator, how many levels of search
//! novelty will be used, etc., depending on the problem characteristics
template <typename FeatureValueT>
class NoveltyFactory {
protected:
	const Problem& _problem;
	
	FSAtomValuationIndexer _indexer;
	
	bool _ignore_neg_literals;
	
	SBFWSConfig::NoveltyEvaluatorType _desired_evaluator_t;
	
	enum class ChosenEvaluatorT {W1Atom, W2Atom, Generic};
	
	//! _chosen_evaluator_t[i] contains the choice of evaluator type for width-i evaluators.
	//! Each time a width-i evaluator is requested, this will be the type os evaluator to be instantiated
	std::vector<ChosenEvaluatorT> _chosen_evaluator_t;
	
	using W1AtomEvaluator = lapkt::novelty::W1AtomEvaluator<FeatureValueT, FSAtomValuationIndexer>;
	using W2AtomEvaluator = lapkt::novelty::W2AtomEvaluator<FeatureValueT, FSAtomValuationIndexer>;
	using CompoundAtomEvaluator = lapkt::novelty::CompoundAtomEvaluator<FeatureValueT, FSAtomValuationIndexer>;
	using GenericEvaluator = lapkt::novelty::GenericNoveltyEvaluator<FeatureValueT>;

	
public:
	using NoveltyEvaluatorT = lapkt::novelty::NoveltyEvaluatorI<FeatureValueT>;
	
	NoveltyFactory(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType desired_evaluator_t, unsigned max_expected_width);
	
	NoveltyEvaluatorT* create_evaluator(unsigned width) const;
	
	NoveltyEvaluatorT* create_compound_evaluator(unsigned max_width) const;
	
protected:
	//! Check whether the size of an optimized atom-evaluator for the given width is small enough,
	//! according to some fixed constants, to make it worthy.
	bool can_use_atom_evaluator(unsigned width) const;
};



} } // namespaces

