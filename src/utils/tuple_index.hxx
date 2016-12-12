
#pragma once

#include <unordered_map>

#include <atom.hxx>

namespace fs0 {

class ProblemInfo;
class Atom;

template <typename Container>
struct container_hash {
    std::size_t operator()(Container const& c) const;
};

class AtomIndex {
protected:
	//! Maps from tuple indexes to their corresponding tuples / atoms
	std::vector<ValueTuple> _tuple_index;
	std::vector<Atom> _atom_index;
	
	//! A map from tuple index to its corresponding symbol
	std::vector<unsigned> _symbol_index;
	
	//! A map from actual tuples to their index
	std::vector<std::unordered_map<ValueTuple, AtomIdx, container_hash<ValueTuple>>> _tuple_index_inv;
	
	//! _atom_index_inv.at(i) contains a map mapping all possible values 'v' of variable 'i'
	//! to the tuple that corresponds to the atom <i, v>
	std::vector<std::unordered_map<ObjectIdx, AtomIdx>> _atom_index_inv;
	
public:
	//! Constructs a full tuple index
	AtomIndex(const ProblemInfo& info);
	AtomIndex(const AtomIndex&) = default;
	AtomIndex(AtomIndex&&) = default;
	AtomIndex& operator=(const AtomIndex& other) = default;
	AtomIndex& operator=(AtomIndex&& other) = default;
	
	//! Return the number of tuples of the index
	unsigned size() const { return _tuple_index.size(); }
	
	//! Add a new element to the index.
	void add(unsigned symbol, const ValueTuple& tuple, unsigned idx, const Atom& atom);

	//! Returns the atom corresponding to the given index
	const Atom& to_atom(AtomIdx tuple) const { return _atom_index.at(tuple); }
	
	//! Returns the index corresponding to the given tuple for the given logical symbol
	AtomIdx to_index(unsigned symbol, const ValueTuple& tuple) const;
	AtomIdx to_index(const std::pair<unsigned, ValueTuple>& tuple) const { return to_index(tuple.first, tuple.second); }
	
	//! Returns the index corresponding to the given atom
	AtomIdx to_index(const Atom& atom) const;
	AtomIdx to_index(VariableIdx variable, ObjectIdx value) const;

	
	//! Returns the actual value tuple that corresponds to the given tuple index, without the logical symbol 
	const ValueTuple& to_tuple(AtomIdx tuple) const { return _tuple_index.at(tuple); }
	
	//! Returns the logical symbol that corresponds to the given tuple index
	unsigned symbol(AtomIdx tuple) const { return _symbol_index.at(tuple); }
	
protected:
	//! A helper to compute and index all reachable tuples.
	//! Returns an index from each logical symbol to all the tuples that are reachable / make sense for that particular
	//! logical symbol.
	static std::vector<std::vector<std::pair<ValueTuple, ObjectIdx>>> compute_all_reachable_tuples(const ProblemInfo& info);
};

} // namespaces
