
#pragma once

#include <cassert>
#include <vector>
#include <unordered_set>
#include <algorithm>


#include <boost/functional/hash.hpp>
#include "features.hxx"
#include "base.hxx"

namespace lapkt { namespace novelty {

//! A tuple of feature valuations of the form
//! X_1 = x_1, X_2 = x_2, ..., X_k = x_k,
//! Where X_i is a feature identifier, and x_i a certain value taken by that feature on some state,
//! is represented by a vector {X_1, x_1, X_2, x_2...} where identifiers and values are contiguous.
class ValueTuple {
public:
	using VariableIndex = int;
	using ValueIndex = int;
	using ContainerT = std::vector<int>;

	ValueTuple() = default;
	~ValueTuple() = default;
	ValueTuple( std::size_t sz);
	ValueTuple(const ValueTuple&) = default;
	ValueTuple(ValueTuple&&) = default;
	ValueTuple& operator=(const ValueTuple&) = default;
	ValueTuple& operator=(ValueTuple&&) = default;


	void add(VariableIndex x, ValueIndex v) {
		elements.push_back(x);
		elements.push_back(v);
	}

	//! Comparison operators
	inline bool operator==( const ValueTuple& t ) const { return elements == t.elements; }
	inline bool operator!=( const ValueTuple& t ) const { return !(operator==(t)); }
	inline bool operator< ( const ValueTuple& t ) const { return elements < t.elements; }
	inline bool operator> ( const ValueTuple& t ) const { return t.operator<(*this); }
	inline bool operator<=( const ValueTuple& t ) const { return !(operator>(t)); }
	inline bool operator>=( const ValueTuple& t ) const { return !(operator<(t)); }


	friend std::ostream& operator<<( std::ostream& os, const ValueTuple& t ) { return t.print(os); }
	std::ostream& print(std::ostream& os) const;

	struct Hasher {
		std::size_t operator()(const ValueTuple& tuple) const;
	};
	
	//! Helper mostly for debugging purposes
	bool _check_ordered() const {
		std::size_t size = elements.size();
		assert(size % 2 == 0);
		
		if (size <= 2)  return true;
		
		for (unsigned i = 2; i < elements.size(); i += 2) {
			if (elements[i-2] > elements[i]) return false;
		}
		return true;
	}

protected:
	ContainerT elements;
};

//! A generic novelty evaluator that accepts all sorts of feature valuations as long as
//! their type is (homogeneously) FeatureValueT
//! The evaluator has somewhat optimized data structures for computing novelty 1 and 2,
//! but is also able to compute higher novelty values.
template <typename FeatureValueT>
class GenericNoveltyEvaluator : public NoveltyEvaluatorI<FeatureValueT> {
public:
	using Base = NoveltyEvaluatorI<FeatureValueT>;
	using ValuationT = typename Base::ValuationT;
	
	GenericNoveltyEvaluator() = delete;
	GenericNoveltyEvaluator(unsigned max_novelty) :
		Base(max_novelty), _tables(max_novelty + 1)
	{}
	
	GenericNoveltyEvaluator* clone() const {
		return new GenericNoveltyEvaluator(*this);
	}
	
	GenericNoveltyEvaluator(const GenericNoveltyEvaluator&) = default;
	GenericNoveltyEvaluator(GenericNoveltyEvaluator&&) = default;
	GenericNoveltyEvaluator& operator=(const GenericNoveltyEvaluator&) = default;
	GenericNoveltyEvaluator& operator=(GenericNoveltyEvaluator&&) = default;


	bool evaluate_width_1_tuples(const ValuationT& current, const std::vector<unsigned>& novel) override;
	bool evaluate_width_2_tuples(const ValuationT& current, const std::vector<unsigned>& novel) override;
	
protected:
	
	//! Check only if the valuation contains a width-'k' tuple which is novel; return k if that is the case, or MAX if not
	unsigned _evaluate(const ValuationT& valuation, const std::vector<unsigned>& novel, unsigned k) override;
	
protected:
	//! Maximum novelty value to be computed
	unsigned _max_novelty;

	
	//! A tuple of width 1 simply contains the identifier of the variable and the value
	using Width1Tuple = std::pair<unsigned, FeatureValueT>;
	using Width2Tuple = std::tuple<unsigned, FeatureValueT, unsigned, FeatureValueT>;
	
	struct Width2TupleHasher { std::size_t operator()(const Width2Tuple& tuple) const; };
	
	using NoveltyTable = std::unordered_set<ValueTuple, ValueTuple::Hasher>;
	using NoveltyTables = std::vector<NoveltyTable>;
	
	std::unordered_set<Width1Tuple, boost::hash<Width1Tuple>> _width_1_tuples;
	std::unordered_set<Width2Tuple, Width2TupleHasher> _width_2_tuples;
	
	NoveltyTables _tables;
};

//! An iterator through all tuples of a certain size that can be derived from a certain vector of values.
//! The iterator returns only those tuples that contain at least _one_ value that is "novel", according to
//! a certain given `novelty` vector that will typically indicate which values are novel wrt the parent search node.
//! For instance, iterating through all size-2 tuples of a valuation {A,B,C} will yield the following (named) tuples:
//!
//! {<0,A>, <1,B>}    (meaning: value A for 0-th element, value B for 1st element)
//! {<0,A>, <2,C>}
//! {<1,B>, <2,C>}
//!
//! If, however, a novel vector {true, false, false} is given, then only the following tuples would be iterated through:
//!
//! {<0,A>, <1,B>}
//! {<0,A>, <2,C>}
//!
//! As it would be implicitly understood that tuple {<1,B>, <2,C>} cannot be new, since both values pertain already
//! to the parent valuation
template <typename FeatureValueT>
class TupleIterator {
public:
	using ValuationT = std::vector<FeatureValueT>;
	
	//! Create an iterator through tuples of size `size` of the given feature valuation.
	TupleIterator(unsigned size, const ValuationT& current, const std::vector<bool>& novel) :
		_current(current), _novel(novel),  _size(size), _indexes(current.size(), false), _ended(false)
	{
		assert(_size > 0);
		assert(current.size() >= _size);
		assert(novel.size() == current.size());
		std::fill(_indexes.begin(), _indexes.begin() + _size, true);
		if (!at_least_one_index_novel()) novel_prev_permutation(); // Seek the first permutation containing at least one novel index.
	}

	~TupleIterator() = default;


	ValueTuple next() {
		assert(!ended());
		// Check http://stackoverflow.com/a/9430993
		unsigned var = 0;
		ValueTuple tuple(_size);
		for (; var < _indexes.size(); ++var) {
			if (_indexes[var]) {
				// std::cout << "(" << var << ", " << _current[i] << ") ";
				tuple.add(var, _current[var]);
			}
		}
		_ended = !novel_prev_permutation();
		return tuple;
	}
	
	bool ended() const {
		return _ended;
	}

protected:
	//! The current valuation from which we want to derive size-k tuples
	const ValuationT& _current;
	
	//! `_novel[i]` iff current[i] != parent[i]
	const std::vector<bool>& _novel;
	
	std::size_t _size;
	
	std::vector<bool> _indexes;
	
	//! Whether the iteration has ended
	bool _ended;

	inline bool at_least_one_index_novel() {
		for (unsigned i = 0; i < _indexes.size(); ++i) {
			if (_novel[i] && _indexes[i]) return true;
		}
		return false;
	}
	
	
	//! Returns false only if there is no previous index permutation such that at least
	//! one of the indexes is novel; otherwise returns true and modifies _indexes with
	//! that permutation
	bool novel_prev_permutation() {
		while (std::prev_permutation(_indexes.begin(), _indexes.end())) {
			if (at_least_one_index_novel()) return true;
		}
		return false;
	}
};

} } // namespaces
