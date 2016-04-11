
#pragma once

#include <unordered_map>
#include <boost/functional/hash.hpp>

#include <atom.hxx>

namespace fs0 {

class ProblemInfo;
class Atom;

template <typename Container>
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};

class TupleIndex {
protected:
	//! Maps from tuple indexes to their corresponding tuples / atoms
	std::vector<ValueTuple> _tuple_index;
	std::vector<Atom> _atom_index;
	
	//! A map from tuple index to its corresponding symbol
	std::vector<unsigned> _symbol_index;
	
	//! A map from actual tuples to their index
	std::vector<std::unordered_map<ValueTuple, TupleIdx, container_hash<ValueTuple>>> _tuple_index_inv;
	
	//! _atom_index_inv.at(i) contains a map mapping all possible values 'v' of variable 'i'
	//! to the tuple that corresponds to the atom <i, v>
	std::vector<std::unordered_map<ObjectIdx, TupleIdx>> _atom_index_inv;
	
public:
	//! Constructs a full tuple index
	TupleIndex(const ProblemInfo& info);
	
	// Disallow copies of the object, as they will be expensive, but allow moves.
	TupleIndex(const TupleIndex&) = delete;
	TupleIndex(TupleIndex&&) = default;
	TupleIndex& operator=(const TupleIndex& other) = delete;
	TupleIndex& operator=(TupleIndex&& other) = default;
	
	//! Return the number of tuples of the index
	unsigned size() const { return _tuple_index.size(); }
	
	//! Add a new element to the index.
	void add(unsigned symbol, const ValueTuple& tuple, unsigned idx, const Atom& atom);

	//! Returns the atom corresponding to the given index
	const Atom& to_atom(TupleIdx tuple) const { return _atom_index.at(tuple); }
	
	//! Returns the index corresponding to the given tuple for the given logical symbol
	TupleIdx to_index(unsigned symbol, const ValueTuple& tuple) const;
	TupleIdx to_index(const std::pair<unsigned, ValueTuple>& tuple) const { return to_index(tuple.first, tuple.second); }
	
	//! Returns the index corresponding to the given atom
	TupleIdx to_index(const Atom& atom) const;
	
	//! Returns the actual value tuple that corresponds to the given tuple index, without the logical symbol 
	const ValueTuple& to_tuple(TupleIdx tuple) const { return _tuple_index.at(tuple); }
	
	//! Returns the logical symbol that corresponds to the given tuple index
	unsigned symbol(TupleIdx tuple) const { return _symbol_index.at(tuple); }
	
	//! A helper to compute and index all reachable tuples
	static std::vector<std::vector<ValueTuple>> compute_all_reachable_tuples(const ProblemInfo& info);
};

} // namespaces
