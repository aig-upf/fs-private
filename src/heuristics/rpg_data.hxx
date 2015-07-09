
#pragma once

#include <unordered_map>
#include <map>
#include <cassert>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <state.hxx>
#include <problem.hxx>

namespace fs0 {

/**
 * A data structure containing book-keeping information concerning the actions that support
 * the achievement of atoms in the Relaxed Planning Graph. This currently includes both
 * the atoms that make an action applicable (in a certain RPG layer) and the "extra"
 * atoms that make a particular effect reachable, i.e. those related to the relevant
 * variables of the effect procedure that achieves the effect.
 */
class RPGData
{
public:
	//! <layer ID, Action ID, action supporting atoms>
	typedef std::tuple<unsigned, ActionIdx, Atom::vctrp> AtomSupport;
	typedef std::map<Atom, AtomSupport> SupportMap;

protected:
	//! This keeps a reference to the novel atoms that have been inserted in the most recent layer of the RPG.
	std::vector<const Atom*> novelAtoms;

	//! The current number of layers.
	unsigned currentLayerIdx;

	/**
	 * A map mapping every atom X=x reached in the RPG layer represented by this changeset to a pair < L, A, V >, where:
	 * - 'L' is the first layer at which the atom has been achieved.
	 * - 'A' is the index of one of the actions that achieves the atom.
	 * - 'V' is a vector with all the atoms that support the achievement of atom X=x through the application of action A.
	 */
	SupportMap _effects;

	//! We keep a pointer to the previous RPG layer to ensure that we only add novel atoms.
	const RelaxedState& _referenceState;

public:
	typedef std::shared_ptr<RPGData> ptr;

	RPGData(const RelaxedState& referenceState) :
		novelAtoms(),
		currentLayerIdx(0),
		_effects(),
		_referenceState(referenceState)
	{};

	~RPGData() {};

	//! Returns the number of layers of the RPG.
	unsigned getNumLayers() const  {return currentLayerIdx + 1; } // 0-indexed!
	
	//! Returns the current layer index
	unsigned getCurrentLayerIdx() const  {return currentLayerIdx; }

	//! Closes the last RPG layer and opens up a new one
	void advanceLayer() {
		novelAtoms.clear();
		++currentLayerIdx;
	}

	//! Returns the support for the given atom
	const AtomSupport& getAtomSupport(const Atom& atom) const {
		auto it = _effects.find(atom);
		assert(it != _effects.end());
		return it->second;
	}

	//! Accumulates all the atoms contained *in the last layer* of the given RPG into the given relaxed state.
	static void accumulate(RelaxedGenericState& state, const RPGData& rpg) {
		for (const Atom* atom:rpg.novelAtoms) {
			state.set(*atom);
		}
	}

	const std::vector<const Atom*>& getNovelAtoms() const { return novelAtoms; }

	//! Returns true iff the given atom is not already tracked by the RPG. In that case, it returns an insertion hint too.
	std::pair<bool, SupportMap::iterator> getInsertionHint(const Atom& atom) {
		if (_referenceState.contains(atom)) 
			return std::make_pair(false, _effects.end()); // Make sure that the atom is novel and relevant

		SupportMap::iterator lb = _effects.lower_bound(atom); // @see http://stackoverflow.com/a/101980
		bool keyInMap = lb != _effects.end() && !(_effects.key_comp()(atom, lb->first));
		if (keyInMap) return std::make_pair(false, _effects.end());
		else return std::make_pair(true, lb);
	}

	//! The version with hint assumes that the atom needs to be inserted.
	void add(const Atom& atom, ActionIdx action, Atom::vctrp causes, SupportMap::iterator hint) {
		// updateEffectMapSimple(fact, RPGraph::pruneSeedSupporters(extraCauses, _seed));
		auto it = _effects.insert(hint, std::make_pair(atom, std::make_tuple(currentLayerIdx, action, causes)));

		// Keep a pointer to the inserted atom. Since the current strategy guarantees that we only insert one atom the first time we encounter it,
		// we can be sure that pointers in the 'novelAtoms' will always be valid and that no pointer will be intended to point to the same atom twice.
		// BEWARE that if we change the update strategy, this might no longer hold.
		novelAtoms.push_back(&(it->first));
	}

	void add(const Atom& atom, ActionIdx action, Atom::vctrp causes) {
		auto hint = getInsertionHint(atom);
		if (!hint.first) return; // Don't insert the atom if it was already tracked by the RPG
		add(atom, action, causes, hint.second);
	}

	friend std::ostream& operator<<(std::ostream &os, const RPGData& data) { return data.print(os); }

	//! Prints a representation of the RPG data to the given stream.
	std::ostream& print(std::ostream& os) const {
		os << "All RPG accumulated atoms (" << _effects.size() << "): ";
		for (const auto& x:_effects) {
			os << x.first  << " (action #" << std::get<1>(x.second) << "), (layer #" << std::get<0>(x.second) << "), (support: ";
			printAtoms(std::get<2>(x.second), os);
			os << "), ";
		}
		os << std::endl;
		return os;
	}

	void printAtoms(const Atom::vctrp vector, std::ostream& os) const {
		for (const auto& fact:*vector) {
			os << fact << ", ";
		}
	}
};


} // namespaces
