
#pragma once

#include <vector>
#include <bits/stl_numeric.h>
#include <limits>
#include <unordered_set>

namespace lapkt { namespace novelty {

//! Discrete novelty values
enum class Novelty { Unknown, One, GTOne, OneAndAHalf, GTOneAndAHalf, Two, GTTwo};

//! A printer for discrete novelty values
std::ostream& operator<<(std::ostream& os, const Novelty& nov);
	
//! Compute a vector with the indexes of those elements in a given valuation that are novel wrt a "parent" valuation.
template <typename FeatureValueT>
std::vector<unsigned> derive_novel(const std::vector<FeatureValueT>& current, const std::vector<FeatureValueT>& parent) {
	std::vector<unsigned> novel;
	for (unsigned i = 0; i < current.size(); ++i) {
		if (current[i] != parent[i]) {
			novel.push_back(i);
		}
	}
	return novel;
}

template <typename FeatureValueT>
class NoveltyEvaluatorI {
public:
	using ValuationT = std::vector<FeatureValueT>;
	
	NoveltyEvaluatorI(unsigned max_novelty) : _max_novelty(max_novelty) {}
	virtual ~NoveltyEvaluatorI() = default;
	virtual NoveltyEvaluatorI* clone() const = 0;

	//! Return the max novelty considered by this evaluator
	unsigned max_novelty() const { return _max_novelty; }
	
	//! Evaluate assuming all elements in the valuation can be novel
	virtual unsigned evaluate(const ValuationT& valuation, unsigned k) = 0;
	
	
	
	bool evaluate_1_5(const ValuationT& valuation, const ValuationT& parent, const std::vector<unsigned>& special) {
		return evaluate_1_5(valuation, derive_novel(valuation, parent), special);
	}
	
	bool evaluate_1_5(const ValuationT& valuation, const std::vector<unsigned>& special) {
		setup_all_features_novel(valuation);
		return evaluate_1_5(valuation, _all_features_novel, special);
	}
	
	virtual bool evaluate_1_5(const ValuationT& valuation, const std::vector<unsigned>& novel, const std::vector<unsigned>& special) {
		throw std::runtime_error("This novelty evaluator is not prepared to compute 1.5 novelty values");
	}
	
	
	
	//! Evaluate assuming all elements in the valuation can be novel
	unsigned evaluate(const ValuationT& valuation) {
		unsigned novelty = std::numeric_limits<unsigned>::max();
		for (unsigned k = 0; k <= _max_novelty; ++k) {
			novelty = std::min(novelty, evaluate(valuation, k));
		}
		return novelty;
	}
	
	
	//! Evaluate the novelty of a node taking into account the valuation of its parent, for optimization purposes
	unsigned evaluate(const ValuationT& valuation, const ValuationT& parent_valuation) {
		std::vector<unsigned> novel = derive_novel(valuation, parent_valuation);
		unsigned novelty = std::numeric_limits<unsigned>::max();
		for (unsigned k = 0; k <= _max_novelty; ++k) {
			novelty = std::min(novelty, _evaluate(valuation, novel, k));
		}
		return novelty;
	}
	
	//! Evaluate the novelty of a node taking into account the valuation of its parent, for optimization purposes
	unsigned evaluate(const ValuationT& valuation, const ValuationT& parent_valuation, unsigned k) {
		std::vector<unsigned> novel = derive_novel(valuation, parent_valuation);
		return _evaluate(valuation, novel, k);
	}
	
	//! Some subclasses might want to override this, that is expected to return all those atoms from the 
	//! last evaluated state that are novel in the sense of belonging to a novel 2-tuple
	virtual void atoms_in_novel_tuple(std::unordered_set<unsigned>&) {}
	virtual void explain(unsigned atom) const {  }
	
	
	virtual void mark_atoms_in_novelty1_table(std::vector<bool>& atoms) const {}
	
protected:
	
	//! Check only if the valuation contains a width-'k' tuple which is novel; return k if that is the case, or MAX if not
	virtual unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel, unsigned k) = 0;
	
	
protected:
	//! The maximum width this evaluator is prepared to handle.
	//! If no particular width is specified, the evaluator computes up to (_max_novelty+1) levels of novelty
	//! (i.e. if _max_novelty=1, then the evaluator will return whether a state has novelty 1 or >1.
	unsigned _max_novelty;
	
	//! This is used to cache a vector <0,1,...,k> of appropriate length and spare the creation of one each time we need it.
	mutable std::vector<unsigned> _all_features_novel;	
	
	void setup_all_features_novel(const ValuationT& valuation) {
		std::size_t num_features = valuation.size();
		if (_all_features_novel.size() != num_features) {
			_all_features_novel.resize(num_features);
			std::iota(_all_features_novel.begin(), _all_features_novel.end(), 0);
		}		
	}
};

} } // namespaces
