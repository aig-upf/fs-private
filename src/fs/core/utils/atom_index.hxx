
#pragma once

#include <unordered_map>
#include <boost/functional/hash.hpp>

#include <fs/core/atom.hxx>

namespace fs0 {

class ProblemInfo;
class Atom;

//! An AtomIndex indexes all possible atoms of a certain problem (be it predicative as in 'clear(b)',
//! or coming from a function, as in 'loc(b,c)'. This essentially means that all
//! possible atoms are assigned a (contiguous) ID, which can be subsequently retrieved in an efficient manner.
//! The inverse operation, i.e. retrieving the index of an atom from the atom itself, can also be performed
//! in as efficient as possible a manner.
class AtomIndex {
protected:
	const ProblemInfo& _info;
	
	const bool _indexes_negated_literals;
	
	//! Maps from tuple indexes to their corresponding tuples / atoms
	std::vector<ValueTuple> _tuple_index;
	std::vector<Atom> _atom_index;
	
	//! A map from tuple index to its corresponding symbol
	std::vector<unsigned> _symbol_index;

	//! A map from variable index to its corresponding symbol
	std::vector<unsigned> _variable_to_symbol;
	
	//! A map from actual tuples to their index
	std::vector<std::unordered_map<ValueTuple, AtomIdx, boost::hash<ValueTuple>>> _tuple_index_inv;
	
	//! _atom_index_inv.at(i) contains a map mapping all possible values 'v' of variable 'i'
	//! to the tuple that corresponds to the atom <i, v>
	std::vector<std::unordered_map<object_id, AtomIdx>> _atom_index_inv;
	
	//! A map from each variable index to all possible atoms that arise from that variable; e.g. for variable
	//! 'loc(b)' with ID 7, _variable_to_atom_index[7] will contain the indexes of atoms loc(b)=a, loc(b)=c, etc.
	//! (currently, note that '_variable_to_atom_index[i]' is the flattened version of _atom_index_inv['i'])
	std::vector<std::vector<AtomIdx>> _variable_to_atom_index;
	
public:
	//! Constructs a full tuple index
	explicit AtomIndex(const ProblemInfo& info, bool index_negated_literals = true);
	AtomIndex(const AtomIndex&) = default;
	AtomIndex(AtomIndex&&) = default;
	AtomIndex& operator=(const AtomIndex& other) = delete;
	AtomIndex& operator=(AtomIndex&& other) = delete;
	
	//! Return the number of tuples of the index
	unsigned size() const { return _tuple_index.size(); }
	
	//! Returns the atom corresponding to the given index
	const Atom& to_atom(AtomIdx tuple) const { return _atom_index.at(tuple); }
	
	//! Returns the index corresponding to the given tuple for the given logical symbol
	AtomIdx to_index(unsigned symbol, const ValueTuple& tuple) const;
	AtomIdx to_index(const std::pair<unsigned, ValueTuple>& tuple) const { return to_index(tuple.first, tuple.second); }
	
	//! Returns the index corresponding to the given atom
	AtomIdx to_index(const Atom& atom) const;
	AtomIdx to_index(VariableIdx variable, const object_id& value) const;
	
	bool is_indexed(VariableIdx variable, const object_id& value) const;

	
	//! Returns the actual value tuple that corresponds to the given tuple index, without the logical symbol 
	const ValueTuple& to_tuple(AtomIdx tuple) const { return _tuple_index.at(tuple); }
	const ValueTuple& to_tuple(VariableIdx variable, const object_id& value) const;

	
	//! Returns the logical symbol that corresponds to the given tuple index
	unsigned symbol(AtomIdx tuple) const { return _symbol_index.at(tuple); }
	unsigned var_to_symbol(VariableIdx variable) const { return _variable_to_symbol.at(variable); }

	
	const std::vector<AtomIdx>& all_variable_atoms(VariableIdx variable) const { return _variable_to_atom_index[variable]; }

	inline bool indexes_negated_literals() const { return _indexes_negated_literals; }
	
protected:
	//! Add a new element to the index.
	void add(const ProblemInfo& info,unsigned symbol, const ValueTuple& tuple, unsigned idx, const Atom& atom);
	
	//! A helper to compute and index all reachable tuples.
	//! Returns an index from each logical symbol to all the tuples that are reachable / make sense for that particular
	//! logical symbol.
	static std::vector<std::vector<std::pair<ValueTuple, object_id>>> compute_all_reachable_tuples(const ProblemInfo& info);
};

} // namespaces
