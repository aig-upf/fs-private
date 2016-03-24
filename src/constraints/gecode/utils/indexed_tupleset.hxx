
#pragma once

#include <gecode/int.hh>
#include <map>

namespace fs0 { namespace gecode {

//! 
class IndexedTupleset {
public:
	typedef std::vector<int> Tuple;
	typedef std::vector<Tuple> TupleVector;
	
	//! Construct a tupleset out of the given tuples
	IndexedTupleset(const TupleVector& tuples);

	IndexedTupleset(const IndexedTupleset&) = default;
	IndexedTupleset(IndexedTupleset&&) = default;
	IndexedTupleset& operator=(const IndexedTupleset&) = default;
	IndexedTupleset& operator=(IndexedTupleset&&) = default;
	

	//! Returns true iff the tupleset is empty
	bool empty() const;
	
	//! Returns the number of tuples in the tupleset
	unsigned size() const;
	
	//! Returns true iff the tupleset contains the given tuple
	bool contains(const Tuple& tuple) const;
	
	Gecode::TupleSet get_tupleset() const;
	
	unsigned get_index(const Tuple& tuple) const;

protected:
	
	//! An index of each possible tuple
	std::map<Tuple, unsigned> _tuples;
	
	//! The gecode tupleset of all tuples + their index
	Gecode::TupleSet _tupleset;

	//! Helper to index all tuples according to their position in the given vector
	static std::map<Tuple, unsigned> index_tuples(const TupleVector& tuples);
	
	//! Helper to construct the actual Gecode tuplesets.
	static Gecode::TupleSet compute_tupleset(const std::map<Tuple, unsigned>& tuple_index);
};


} } // namespaces

