
#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

#include "base.hxx"
#include <aptk2/tools/logging.hxx>


namespace lapkt { namespace novelty {

uint32_t _combine_indexes(uint32_t index1, uint32_t index2, uint32_t num_atom_indexes);


template <typename FeatureValueT, typename ValuationIndexerT, typename Tuple2MarkerT>
class AtomNoveltyEvaluator : public NoveltyEvaluatorI<FeatureValueT> {
public:
	using Base = NoveltyEvaluatorI<FeatureValueT>;
	using ValuationT = typename Base::ValuationT;


	//! Factory method - creates the evaluator only if suitable, i.e. if the number of atoms of the problem is small enough
	static AtomNoveltyEvaluator* create(const ValuationIndexerT& indexer, bool ignore_negative) {

		// With this novelty evaluator we can handle up to 2^32 atoms
		if (indexer.num_indexes() >= 65536) return nullptr;

		if (!Tuple2MarkerT::can_handle(num_combined_indexes(indexer.num_indexes()))) return nullptr;

		return new AtomNoveltyEvaluator(indexer, ignore_negative);
	}

	AtomNoveltyEvaluator() = delete;

protected:
	AtomNoveltyEvaluator(const ValuationIndexerT& indexer, bool ignore_negative) :
		_indexer(indexer),
		_ignore_negative(ignore_negative),
		_num_atom_indexes(_indexer.num_indexes()),
		_seen_tuples_sz_1(_num_atom_indexes, false),
		_t2marker(num_combined_indexes(), _num_atom_indexes)
	{}

public:

	AtomNoveltyEvaluator* clone() const override {
		return new AtomNoveltyEvaluator(*this);
	}

	AtomNoveltyEvaluator(const AtomNoveltyEvaluator&) = default;
	AtomNoveltyEvaluator(AtomNoveltyEvaluator&&) = default;
	AtomNoveltyEvaluator& operator=(const AtomNoveltyEvaluator&) = default;
	AtomNoveltyEvaluator& operator=(AtomNoveltyEvaluator&&) = default;

	bool evaluate_width_1_tuples(const ValuationT& valuation, const std::vector<unsigned>& novel) override {
		bool exists_novel_tuple = false;
		for (unsigned var_index:novel) {
			const auto& feature_value = valuation[var_index];
			if (_ignore_negative && feature_value == 0) continue;

			unsigned atom_index = _indexer.to_index(var_index, feature_value);
			//auto&& value = _seen_tuples_sz_1[atom_index]; // see http://stackoverflow.com/a/8399942
			// MRJ: Looks to me the above was needed probably with c++ compilers available circa
			// 2012, with g++ 5.4.0 the original code works just fine
			std::vector< bool >::reference value = _seen_tuples_sz_1[atom_index];
			if (!value) { // The tuple is new, hence the novelty of the state is 1
				exists_novel_tuple = true;
				value = true;
			}
		}
		return exists_novel_tuple;
	}

	bool evaluate_width_2_tuples(const ValuationT& valuation, const std::vector<unsigned>& novel) override {
		unsigned sz = valuation.size();
		if (sz == novel.size()) return _evaluate_width_2_tuples(valuation);

		bool exists_novel_tuple = false;

		for (unsigned i = 0; i < novel.size(); ++i) {
			unsigned idx1 = novel[i];
			const auto& feature1_value = valuation[idx1];
			if (_ignore_negative && feature1_value == 0) continue;

			unsigned atom1_index = _indexer.to_index(idx1, feature1_value);

			for (unsigned j = 0; j < sz; ++j) {
				if (j==idx1) continue;

				const auto& feature2_value = valuation[j];
				if (_ignore_negative && feature2_value == 0) continue;

				exists_novel_tuple |= _t2marker.update_sz2_table(atom1_index, _indexer.to_index(j, feature2_value));
			}
		}
		return exists_novel_tuple;
	}

	bool _evaluate_width_2_tuples(const ValuationT& valuation) {
		bool exists_novel_tuple = false;
		unsigned sz = valuation.size();

		for (unsigned i = 0; i < sz; ++i) {
			unsigned atom1_index = _indexer.to_index(i, valuation[i]);

			for (unsigned j = i+1; j < sz; ++j) {
				const auto& feature2_value = valuation[j];
				if (_ignore_negative && feature2_value == 0) continue;
				exists_novel_tuple |= _t2marker.update_sz2_table(atom1_index, _indexer.to_index(j, feature2_value));
			}
		}
		return exists_novel_tuple;
	}

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


protected:
	ValuationIndexerT _indexer;

	bool _ignore_negative;

	uint32_t _num_atom_indexes;

	std::vector<bool> _seen_tuples_sz_1;

	Tuple2MarkerT _t2marker;


	unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel) override {
		throw std::runtime_error("Unsupported type of novelty evaluation operation");
	};

	unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel, unsigned k) override {
		assert(!valuation.empty());
		assert(k==1 || k==2);

		unsigned novelty = std::numeric_limits<unsigned>::max();
		if (k == 1) {
			if (evaluate_width_1_tuples(valuation, novel)) novelty = 1;
		} else if (k == 2) {
			if (evaluate_width_2_tuples(valuation, novel)) novelty = 2;
		}

		return novelty;
	}
};


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

	//! Return the approx. expected size (in bytes) of novelty-2 table.
	static uint64_t expected_size(unsigned num_combined_indexes) {
		return num_combined_indexes / 8;
	}

protected:
	std::vector<bool> _seen_tuples_sz_2;

	unsigned _num_atom_indexes;
};


} } // namespaces
