
#pragma once

#include <gecode/int.hh>
#include <map>
#include <fs0_types.hxx>

namespace fs0 { namespace gecode {

//! 
class IndexedTupleset {
public:
	typedef std::vector<ValueTuple> TupleVector;
	
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
	bool contains(const ValueTuple& tuple) const;
	
	Gecode::TupleSet get_tupleset() const;
	
	unsigned get_index(const ValueTuple& tuple) const;

protected:
	
	//! An index of each possible tuple
	std::map<ValueTuple, unsigned> _tuples;
	
	//! The gecode tupleset of all tuples + their index
	Gecode::TupleSet _tupleset;

	//! Helper to index all tuples according to their position in the given vector
	static std::map<ValueTuple, unsigned> index_tuples(const TupleVector& tuples);
	
	//! Helper to construct the actual Gecode tuplesets.
	static Gecode::TupleSet compute_tupleset(const std::map<ValueTuple, unsigned>& tuple_index);
};


} } // namespaces

