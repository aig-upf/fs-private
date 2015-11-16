
#pragma once

#include <map>
#include <fs0_types.hxx>
#include <atom.hxx>


namespace fs0 {

class State;
class ActionID;

/**
 * A data structure containing book-keeping information concerning the actions that support
 * the achievement of atoms in the Relaxed Planning Graph. This currently includes both
 * the atoms that make an action applicable (in a certain RPG layer) and the "extra"
 * atoms that make a particular effect reachable, i.e. those related to the relevant
 * variables of the effect procedure that achieves the effect.
 */
class RPGData {
public:
	//! <layer ID, Action ID, support>
	typedef std::tuple<unsigned, const ActionID*, Atom::vctrp> AtomSupport;
	typedef std::map<Atom, AtomSupport> SupportMap;

protected:
	//! This keeps a reference to the novel atoms that have been inserted in the most recent layer of the RPG.
	std::vector<std::vector<ObjectIdx>> _novel;
	unsigned _num_novel;

	//! The current number of layers.
	unsigned _current_layer;

	/**
	 * A map mapping every atom X=x reached in the RPG layer represented by this changeset to a pair < L, A, V >, where:
	 * - 'L' is the first layer at which the atom has been achieved.
	 * - 'A' is the index of one of the actions that achieves the atom.
	 * - 'V' is a vector with all the atoms that support the achievement of atom X=x through the application of action A.
	 */
	SupportMap _effects;

public:
	RPGData(const State& seed);
	~RPGData();

	//! Returns the number of layers of the RPG.
	unsigned getNumLayers() const  {return _current_layer + 1; } // 0-indexed!
	
	//! Returns the current layer index
	unsigned getCurrentLayerIdx() const  {return _current_layer; }

	//! Closes the last RPG layer and opens up a new one
	void advanceLayer();
	
	//! Creates an atom support data structure with the given data and taking into account the current RPG layer
	AtomSupport createAtomSupport(const ActionID* action, Atom::vctrp support) const;

	//! Returns the support for the given atom
	const AtomSupport& getAtomSupport(const Atom& atom) const;

	//! Get the number of novel atoms in the last layer of the RPG
	unsigned getNumNovelAtoms() const { return _num_novel; }
	
	const std::vector<std::vector<ObjectIdx>>& getNovelAtoms() const { return _novel; }


	//! Returns a pair "<b, it>" such that b is true iff the given atom is not already tracked by the RPG.
	//! In that case, 'it' is an iterator that can be used as an insertion hint
	//! If, on the other hand, b is false, 'it' is a valid iterator pointing to the atom support
	std::pair<bool, SupportMap::iterator> getInsertionHint(const Atom& atom);
	
	//! The version with hint assumes that the atom needs to be inserted.
	void add(const Atom& atom, const ActionID* action, Atom::vctrp support, SupportMap::iterator hint);
	
	//! Add an atom to the set of newly-reached atoms, only if it is indeed new.
	void add(const Atom& atom, const ActionID* action, Atom::vctrp support);
	
	//! Compute the sum of h_max values of all the given atoms, assuming that they have already been reached in the RPG data structure
	unsigned compute_hmax_sum(const std::vector<Atom>& atoms) const;

	friend std::ostream& operator<<(std::ostream &os, const RPGData& data) { return data.print(os); }

	//! Prints a representation of the RPG data to the given stream.
	std::ostream& print(std::ostream& os) const;

	void printAtoms(const Atom::vctrp vector, std::ostream& os) const;
};


} // namespaces
