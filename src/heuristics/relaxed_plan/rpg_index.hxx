
#pragma once

#include <gecode/int.hh>
#include <fs_types.hxx>
#include <unordered_map>
#include <unordered_set>


namespace fs0 { class ProblemInfo; class State; class Atom; class ActionID; class TupleIndex; }

namespace fs0 { namespace gecode {
	
class ExtensionHandler;


/**
 * A data structure containing book-keeping information concerning the actions that support
 * the achievement of atoms in the Relaxed Planning Graph. This currently includes both
 * the atoms that make an action applicable (in a certain RPG layer) and the "extra"
 * atoms that make a particular effect reachable, i.e. those related to the relevant
 * variables of the effect procedure that achieves the effect.
 */
class RPGIndex {
public:
	//! <layer ID, Action ID, support>, where 'support' is a vector of indexes of logical symbol tuples
	typedef std::tuple<unsigned, const ActionID*, std::vector<TupleIdx>> TupleSupport;
	
	//! A map from the index of a logical symbol tuple to its support in the RPG
// 	typedef std::unordered_map<TupleIdx, TupleSupport> SupportMap;
	typedef std::vector<TupleSupport*> SupportMap;

protected:
	/**
	 * A map with all tuples that have been reached in the RPG. Each tuple index 'I' is mapped to a pair < L, A, V >, where:
	 * - 'L' is the first layer at which the atom has been achieved.
	 * - 'A' is the index of one of the actions that achieves the atom.
	 * - 'V' is a vector with the indexes of all tuples that support the achievement of tuple 'I' through the application of action A.
	 */
	SupportMap _reached;

	//! This keeps a reference to the novel atoms that have been inserted in the most recent layer of the RPG.
	std::vector<TupleIdx> _novel_tuples;

	//! The current number of layers.
	unsigned _current_layer;
	
	ExtensionHandler& _extension_handler;
	
	//! The allowed values in the relation that corresponds to every predicate
	std::vector<Gecode::TupleSet> _extensions;
	
	//! The set of reached values for every state variable
	std::vector<Gecode::IntSet> _domains;
	
	//! This is the set of all values reached so far for each state variable
	std::vector<std::vector<ObjectIdx>> _domains_raw;
	
	const TupleIndex& _tuple_index;
	
	const State& _seed;

public:
	explicit RPGIndex(const State& seed, const TupleIndex& tuple_index, ExtensionHandler& extension_handler);
	~RPGIndex();
	
	//! Returns true if the given tuple has already been reached in the current graph.
	bool reached(TupleIdx tuple) const;
	
	bool is_true(VariableIdx variable) const;
	const Gecode::TupleSet& get_extension(unsigned symbol_id) const { return _extensions.at(symbol_id); }
	const std::vector<Gecode::IntSet>& get_domains() const { return _domains; }
	const Gecode::IntSet& get_domain(VariableIdx variable) const { return _domains.at(variable); }

	//! Returns the number of layers of the RPG.
	unsigned getNumLayers() const  {return _current_layer + 1; } // 0-indexed!
	
	//! Returns the current layer index
	unsigned getCurrentLayerIdx() const  {return _current_layer; }

	//! Returns the support for the given atom
	const TupleSupport& getTupleSupport(TupleIdx tuple) const;
	
	const State& getSeed() const { return _seed; }

	//!
	bool hasNovelTuples() const { return !_novel_tuples.empty(); }
	
	std::size_t num_novel_tuples() const { return _novel_tuples.size(); }
	
	
	const std::vector<TupleIdx>& getNovelTuples() const { return _novel_tuples; }

	//!
	void advance();
	
	
	//! Add an atom to the set of newly-reached atoms, only if it is indeed new.
	void add(TupleIdx tuple, const ActionID* action, std::vector<TupleIdx>&& support);
	
	//! Compute the sum of h_max values of all the given atoms, assuming that they have already been reached in the RPG data structure
// 	unsigned compute_hmax_sum(const std::vector<Atom>& atoms) const;

	//! Prints a representation of the RPG data to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const RPGIndex& data) { return data.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	//! Return the set of all tuples that have not been yet reached in the current RPG.
	std::vector<bool> achieved_atoms(const TupleIndex& tuple_index) const;
	
// 	const std::set<unsigned>& get_modified_symbols() const;


protected:
	//! Creates an atom support data structure with the given data and taking into account the current RPG layer
	TupleSupport* createTupleSupport(const ActionID* action, std::vector<TupleIdx>&& support) const;
	
	void printAtoms(const std::vector<TupleIdx>& vector, std::ostream& os) const;
	
	void next();
};


} } // namespaces
