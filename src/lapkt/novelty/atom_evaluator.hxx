
#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

#include "base.hxx"
#include <aptk2/tools/logging.hxx>
#include <utils/utils.hxx>



namespace lapkt { namespace novelty {

uint32_t _combine_indexes(uint32_t index1, uint32_t index2, uint32_t num_atom_indexes);


template <typename FeatureValueT, typename ValuationIndexerT, typename Tuple2MarkerT>
class AtomNoveltyEvaluator : public NoveltyEvaluatorI<FeatureValueT> {
public:
	using Base = NoveltyEvaluatorI<FeatureValueT>;
	using ValuationT = typename Base::ValuationT;


	//! Factory method - creates the evaluator only if suitable, i.e. if the number of atoms of the problem is small enough
	static AtomNoveltyEvaluator* create(const ValuationIndexerT& indexer, bool ignore_negative, unsigned max_width) {
		if (max_width > 2) return nullptr; // This evaluator is not prepared for such high widths
		
		if (max_width == 2) { // Some additional checks that are not necessary if all we'll do is to compute width 1.
			// With this novelty evaluator we can handle up to 2^32 atoms
			if (indexer.num_indexes() >= 65536) return nullptr;

			if (!Tuple2MarkerT::can_handle(num_combined_indexes(indexer.num_indexes()))) return nullptr;
		}

		return new AtomNoveltyEvaluator(indexer, ignore_negative, max_width);
	}

	AtomNoveltyEvaluator() = delete;

protected:
	//! The indexer that maps each pair of (feature, value) to a feature-index.
	ValuationIndexerT _indexer;

	//! Whether we want to ignore "negative" values, i.e. values of 0.
	bool _ignore_negative;

	//! The total number of possible feature-indexes that can be given by the feature valuation indexer.
	uint32_t _num_atom_indexes;

	//! The tuples of size 1 that we have seen so far
	std::vector<bool> _seen_tuples_sz_1;

	//! The tuple-2 marker stores in a policy-dependent manner the information
	//! about the tuples of size 2 that we have seen so far
	Tuple2MarkerT _t2marker;
	
	AtomNoveltyEvaluator(const ValuationIndexerT& indexer, bool ignore_negative, unsigned max_width) :
		Base(max_width),
		_indexer(indexer),
		_ignore_negative(ignore_negative),
		_num_atom_indexes(_indexer.num_indexes()),
		_seen_tuples_sz_1(_num_atom_indexes, false),
		_t2marker(num_combined_indexes(), _num_atom_indexes)
	{}

public:

	AtomNoveltyEvaluator(const AtomNoveltyEvaluator&) = default;
	AtomNoveltyEvaluator(AtomNoveltyEvaluator&&) = default;
	AtomNoveltyEvaluator& operator=(const AtomNoveltyEvaluator&) = default;
	AtomNoveltyEvaluator& operator=(AtomNoveltyEvaluator&&) = default;
	AtomNoveltyEvaluator* clone() const override { return new AtomNoveltyEvaluator(*this); }
	

	inline unsigned num_combined_indexes() const { return num_combined_indexes(_num_atom_indexes); }

	static unsigned num_combined_indexes(unsigned num_atom_indexes) {
		// If we can have atom indexes in the range [0.._num_atom_indexes-1], then the highest combined index
		// we can have is combine_indexes(_num_atom_indexes-1, _num_atom_indexes-2)
		return _combine_indexes(num_atom_indexes-1, num_atom_indexes-2, num_atom_indexes);
	}

	//! Return the approx. expected size (in bytes) of the two novelty tables.
	uint64_t expected_size() { return expected_size1() + expected_size2(); }

	//! Return the approx. expected size (in bytes) of novelty-1 table.
	uint64_t expected_size1() {
		return _num_atom_indexes / 8; // Assuming a vector of bools packs 8 bools into a single byte.
	}

	//! Return the approx. expected size (in bytes) of novelty-2 table.
	uint64_t expected_size2() {
		return _t2marker.expected_size(num_combined_indexes());
	}

	//! Evaluate assuming all elements in the valuation can be novel
	unsigned evaluate(const ValuationT& valuation, unsigned k) override {
		assert(!valuation.empty());

		if (k == 1) {
			return evaluate_width_1_tuples(valuation) ? 1 : std::numeric_limits<unsigned>::max();
		}
		
		assert(k==2);
		return evaluate_pairs(valuation) ? 2 : std::numeric_limits<unsigned>::max();
	}

protected:

	unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel, unsigned k) override {
		assert(!valuation.empty());

		if (k == 1) {
			return evaluate_width_1_tuples(valuation, novel) ? 1 : std::numeric_limits<unsigned>::max();
		}
		
		assert(k==2);
		return evaluate_pairs(valuation, novel) ? 2 : std::numeric_limits<unsigned>::max();
	}
	
	bool evaluate_width_1_tuples(const ValuationT& valuation, const std::vector<unsigned>& novel) {
		bool exists_novel_tuple = false;
		for (unsigned var_index:novel) {
			exists_novel_tuple |= update_tuple1(var_index, valuation[var_index]);
		}
		return exists_novel_tuple;
	}
	
	//! Assume all elements in the valuation can be new.
	//! NOTE this closely mirrors the code of the method with the `novel` parameter,
	//! but we favor here performance over avoiding code duplication.
	bool evaluate_width_1_tuples(const ValuationT& valuation) {
		bool exists_novel_tuple = false;
		for (unsigned var_index = 0; var_index < valuation.size(); ++var_index) {
			exists_novel_tuple |= update_tuple1(var_index, valuation[var_index]);
		}
		return exists_novel_tuple;
	}	

	bool evaluate_pairs(const ValuationT& valuation, const std::vector<unsigned>& novel) {
		if(this->_max_novelty < 2) {  // i.e. make sure the evaluator was prepared for this width!
			throw std::runtime_error("The AtomNoveltyEvaluator was not prepared for width-2 computation. You need to invoke the creator with max_width=2");
		}
		
		assert(valuation.size() >= novel.size());
		
		if (valuation.size() == novel.size()) return evaluate_pairs(valuation); // Just in case
		

		// WORK-IN-PROGRESS
// 		std::vector<unsigned> novel_indexes, non_novel_indexes;
// 		novel_indexes.reserve(novel_sz); non_novel_indexes.reserve(novel_sz);
		
		
		auto all_indexes = index_valuation(valuation);
		auto novel_indexes = index_valuation(novel, valuation);

		bool exists_novel_tuple = false;
		for (unsigned feat_index1:novel_indexes) {
			for (unsigned feat_index2:all_indexes) {
				if (feat_index1==feat_index2) continue;
				exists_novel_tuple |= _t2marker.update_sz2_table(feat_index1, feat_index2);
			}
		}
		return exists_novel_tuple;
	}	
	
	

	bool evaluate_pairs(const ValuationT& valuation) {
		return evaluate_pairs_from_index(index_valuation(valuation));
	}

	//! Evaluate all pairs from a vector with all feature value indexes.
	bool evaluate_pairs_from_index(const std::vector<unsigned>& indexes) {
		bool exists_novel_tuple = false;
		unsigned sz = indexes.size();

		for (unsigned i = 0; i < sz; ++i) {
			unsigned index_i = indexes[i];

			for (unsigned j = i+1; j < sz; ++j) {
				exists_novel_tuple |= _t2marker.update_sz2_table(index_i, indexes[j]);
			}
		}
		return exists_novel_tuple;
	}	

	//! Helper. Map a feature valuation into proper indexes. Ignore negative values if so requested.
	std::vector<unsigned> index_valuation(const ValuationT& valuation) {
		unsigned sz = valuation.size();
		std::vector<unsigned> indexes;
		indexes.reserve(sz);
		for (unsigned i = 0; i < sz; ++i) {
			const auto& value = valuation[i];
			if (_ignore_negative && value == 0) continue;
			indexes.push_back(_indexer.to_index(i, value));
		}
		return indexes;
	}
	
	//! This one performs the same mapping but assuming we only want the values given by the indexes in 'novel'
	std::vector<unsigned> index_valuation(const std::vector<unsigned>& novel, const ValuationT& valuation) {
		std::vector<unsigned> indexes;
		indexes.reserve(novel.size());
		for (unsigned i:novel) {
			const auto& value = valuation[i];
			if (_ignore_negative && value == 0) continue;
			indexes.push_back(_indexer.to_index(i, value));
		}
		return indexes;
	}	
	
	//! Helper. Returns true if the given feature is novel in the index of 1-tuples.
	bool update_tuple1(unsigned index, const FeatureValueT& value) {
		if (_ignore_negative && value == 0) return false;

		unsigned atom_index = _indexer.to_index(index, value);
		std::vector<bool>::reference ref = _seen_tuples_sz_1[atom_index];
		if (!ref) { // The tuple is new
			ref = true;
			return true;
		}
		return false;
	}
	
	
	
	
	
	bool evaluate_width_1_and_half_tuples(const ValuationT& valuation, const std::vector<unsigned>& novel, const std::vector<unsigned>& special) {
		if(this->_max_novelty < 2) {  // i.e. make sure the evaluator was prepared for this width!
			throw std::runtime_error("The AtomNoveltyEvaluator was not prepared for width-2 computation. You need to invoke the creator with max_width=2");
		}
		
		_t2marker.start();
		
		unsigned sz = valuation.size();
// 		if (sz == novel.size()) return _evaluate_width_2_tuples(valuation);

		
		// Compute intersection and set differences in one pass
		std::vector<unsigned> intersect, novel_wo_special, special_wo_novel;
		fs0::Utils::intersection_and_set_diff(novel, special, intersect, novel_wo_special, special_wo_novel);
		
		bool exists_novel_tuple = false;
		
		
		// 1. Check if there is a novel pair involving one element in the intersection plus any other element 
		for (unsigned idx1:intersect) {
			const auto& feature1_value = valuation[idx1];
			if (_ignore_negative && feature1_value == 0) continue;
			unsigned atom1_index = _indexer.to_index(idx1, feature1_value);

			for (unsigned j = 0; j < sz; ++j) {
				if (j==idx1) continue;

				const auto& feature2_value = valuation[j];
				if (_ignore_negative && feature2_value == 0) continue;
				exists_novel_tuple  |= _t2marker.update_sz2_table(atom1_index, _indexer.to_index(j, feature2_value));
			}
		}
		
		
		// 2. Check if there is a novel pair involving one element from novel \ special and one element from special \ novel
		for (unsigned idx1:novel_wo_special) {
			const auto& feature1_value = valuation[idx1];
			if (_ignore_negative && feature1_value == 0) continue;
			unsigned atom1_index = _indexer.to_index(idx1, feature1_value);

			for (unsigned idx2:special_wo_novel) {
				if (idx1==idx2) continue;

				const auto& feature2_value = valuation[idx2];
				if (_ignore_negative && feature2_value == 0) continue;
				exists_novel_tuple  |= _t2marker.update_sz2_table(atom1_index, _indexer.to_index(idx2, feature2_value));
			}
		}		
		
		return exists_novel_tuple;
	}
	
	
	void atoms_in_novel_tuple(std::unordered_set<unsigned>& atoms) override {
		_t2marker.atoms_in_novel_tuple(atoms);
	}
	
	virtual void explain(unsigned atom) const { _t2marker.explain(atom); }
	
	
	
};

//! A 2-tuple marker based on a large std::vector of bools that keeps, for each possible index
//! that represents the combination of two novelty feature indexes, whether that index has been
//! previously seen or not.
class BoolVectorTuple2Marker {
public:
	static bool can_handle(unsigned num_combined_indexes) {
		return expected_size(num_combined_indexes) < 10000000; // i.e. max 10MB per table.
	}

	BoolVectorTuple2Marker(unsigned num_combined_indexes, unsigned num_atom_indexes) :
		_seen_tuples_sz_2(num_combined_indexes, false),
		_num_atom_indexes(num_atom_indexes)
	{
		LPT_INFO("cout", "Created a Novelty-2 table of approx. size " << expected_size(num_combined_indexes) / 1024 << "KB");
	}
	
	virtual ~BoolVectorTuple2Marker() = default;
	
	virtual void start() {} // Required by template	

	bool update_sz2_table(unsigned atom1_index, unsigned atom2_index) {
		uint32_t combined = _combine_indexes(atom1_index, atom2_index, _num_atom_indexes);
		assert(combined < _seen_tuples_sz_2.size());
		auto&& value = _seen_tuples_sz_2[combined]; // see http://stackoverflow.com/a/8399942
		if (!value) { // The tuple is new
			value = true;
			return true;
		}
		return false;
	}

	virtual void atoms_in_novel_tuple(std::unordered_set<unsigned>& atoms) {}
	virtual void explain(unsigned atom) const {}
	
	//! Return the approx. expected size (in bytes) of novelty-2 table.
	static uint64_t expected_size(unsigned num_combined_indexes) {
		return num_combined_indexes / 8;
	}

protected:
	std::vector<bool> _seen_tuples_sz_2;

	unsigned _num_atom_indexes;
};


//! Same than the parent class, but stores exactly those atoms in each state are part of a novel 2-tuple of atoms
class PersistentBoolVectorTuple2Marker : public BoolVectorTuple2Marker {
public:
	PersistentBoolVectorTuple2Marker(unsigned num_combined_indexes, unsigned num_atom_indexes) : BoolVectorTuple2Marker(num_combined_indexes, num_atom_indexes) {}

	void start() override {
		_part_of_a_novel_tuple.clear();
// 		_explanation.clear();
	}
	
	bool update_sz2_table(unsigned atom1_index, unsigned atom2_index) {
		uint32_t combined = _combine_indexes(atom1_index, atom2_index, _num_atom_indexes);
		assert(combined < _seen_tuples_sz_2.size());
		std::vector<bool>::reference value = _seen_tuples_sz_2[combined];
		if (!value) { // The tuple is new
			value = true;
			_part_of_a_novel_tuple.insert(atom1_index);
			_part_of_a_novel_tuple.insert(atom2_index);
// 			_explanation.insert(std::make_pair(atom1_index, atom2_index));
// 			_explanation.insert(std::make_pair(atom2_index, atom1_index));
			return true;
		}
		return false;
	}
	
	void atoms_in_novel_tuple(std::unordered_set<unsigned>& atoms) override {
		atoms = std::move(_part_of_a_novel_tuple);
		_part_of_a_novel_tuple = std::unordered_set<unsigned>();
	}
	
	void explain(unsigned atom) const {
// 		auto it = _explanation.find(atom);
// 		assert(it != _explanation.end());
// 		LPT_INFO("cout", "Atom " << atom << " is part of the novel 2-tuple: (" <<  atom << ", " << it->second  << ")");
	}



protected:
	std::unordered_set<unsigned> _part_of_a_novel_tuple;
// 	std::unordered_map<unsigned, unsigned> _explanation;
};



} } // namespaces
