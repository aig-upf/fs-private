
#pragma once

#include <unordered_map>
#include <map>
#include <cassert>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <state.hxx>


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
	typedef std::tuple<unsigned, ActionIdx, Atom::vctrp> AtomSupport;
	
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
	std::map<Atom, AtomSupport> _effects;
	
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
	unsigned getNumLayers() const  {
		return currentLayerIdx + 1; // 0-indexed!
	}
	
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


	
	virtual void add(const Atom& atom, ActionIdx action, Atom::vctrp causes) {
		if (_referenceState.contains(atom)) return; // Make sure that the atom is novel.
		
		auto lb = _effects.lower_bound(atom); // @see http://stackoverflow.com/a/101980
		if(lb != _effects.end() && !(_effects.key_comp()(atom, lb->first))) { // The key is already on the map
			// ATM, if that is the case we simply stick to the already-inserted atom, for simplicity and efficiency. Before changing this strategy read the the rest of the comments in this method!
			// A possible heuristic alternative would be to update the support for the atom if the new support set contains a smaller number of supports not present in the seed state.
			// unsigned other_total_non_seed_supporters = std::get<2>(lb->second);
			// if (causes.size() < other_total_non_seed_supporters) { 
			// 	lb->second = std::make_tuple(_activeActionIdx, causes, causes.size());
			// }
		} else {
			// updateEffectMapSimple(fact, RPGraph::pruneSeedSupporters(extraCauses, _seed));
			auto it = _effects.insert(lb, std::make_pair(atom, std::make_tuple(currentLayerIdx, action, causes)));
			
			// Keep a pointer to the inserted atom. Since the current strategy guarantees that we only insert one atom the first time we encounter it,
			// we can be sure that pointers in the 'novelAtoms' will always be valid and that no pointer will be intended to point to the same atom twice.
			// BEWARE that if we change the update strategy, this might no longer hold.
			novelAtoms.push_back(&(it->first));
		}
	}
	
	friend std::ostream& operator<<(std::ostream &os, const RPGData& data) { return data.print(os); }
	
	//! Prints a representation of the RPG data to the given stream.
	std::ostream& print(std::ostream& os) const {
		os << "All RPG accumulated atoms: ";
		for (const auto& x:_effects) {
			os << x.first  << " (action #" << std::get<1>(x.second) << "), (layer #" << std::get<0>(x.second) << ") (support: ";
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
