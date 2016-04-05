
#pragma once

#include <atom.hxx>
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace fs0 {

class ProblemInfo;

template <typename Container> // we can make this generic for any container [1]
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};

class TupleIndex {
protected:
	
// 	std::unordered_map<std::pair<unsigned, ValueTuple>, TupleIdx> _inverse;
	std::vector<std::unordered_map<ValueTuple, TupleIdx, container_hash<ValueTuple>>> _inverse;
	
	std::vector<ValueTuple> _index;
	std::vector<unsigned> _symbol_index;
	
	//! The atom corresponding to each tuple index
	std::vector<Atom> _tuple_atoms;
	std::map<Atom, TupleIdx> _tuple_atoms_inv;
	
public:
	//! Constructs a full tuple index
	TupleIndex(const ProblemInfo& info);
	
	// Disallow copies of the object, as they will be expensive, but allow moves.
	TupleIndex(const TupleIndex&) = delete;
	TupleIndex(TupleIndex&&) = default;
	TupleIndex& operator=(const TupleIndex& other) = delete;
	TupleIndex& operator=(TupleIndex&& other) = default;
	
	//! Return the number of tuples of the index
	unsigned size() const { return _index.size(); }
	
	void add(unsigned symbol, const ValueTuple& tuple, unsigned idx, const Atom& atom);
	
	const Atom& to_atom(TupleIdx tuple) const { return _tuple_atoms.at(tuple); }
	
	TupleIdx to_index(const std::pair<unsigned, ValueTuple>& tuple) const { return to_index(tuple.first, tuple.second); }
	
	TupleIdx to_index(unsigned symbol, const ValueTuple& tuple) const {
		const auto& map = _inverse.at(symbol);
		auto it = map.find(tuple);
		assert(it != map.end());
		return it->second;
	}
	
	TupleIdx to_index(const Atom& atom) const { 
		auto it = _tuple_atoms_inv.find(atom);
		assert(it != _tuple_atoms_inv.end());
		return it->second;
	}
	
	//! Returns the actual value tuple that corresponds to the given tuple index, without the logical symbol 
	const ValueTuple& from_index(TupleIdx tuple) const { return _index.at(tuple); }
	
	//! Returns the logical symbol that corresponds to the given tuple index
	unsigned symbol(TupleIdx tuple) const { return _symbol_index.at(tuple); }
	
	//!
	static std::vector<std::vector<ValueTuple>> compute_all_reachable_tuples(const ProblemInfo& info);
};

} // namespaces
