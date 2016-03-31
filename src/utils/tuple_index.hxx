
#pragma once

#include <atom.hxx>

namespace fs0 {

class ProblemInfo;

class TupleIndex {
protected:
	
	std::map<std::pair<unsigned, ValueTuple>, TupleIdx> _inverse;
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
	
	
	void add(unsigned symbol, const ValueTuple& tuple, unsigned idx, const Atom& atom);
	
	const Atom& to_atom(TupleIdx tuple) const { return _tuple_atoms.at(tuple); }
	
	TupleIdx to_index(unsigned symbol, const ValueTuple& tuple) const { return to_index(std::make_pair(symbol, tuple)); }
	
	TupleIdx to_index(const std::pair<unsigned, ValueTuple>& tuple) const { 
		auto it = _inverse.find(tuple);
		assert(it != _inverse.end());
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
