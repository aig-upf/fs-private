
#pragma once

#include <unordered_map>
#include <map>
#include <cassert>
#include <fs0_types.hxx>
#include <atom.hxx>
#include <problem.hxx>

namespace fs0 {

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
	typedef std::tuple<unsigned, ActionIdx, Atom::vctrp> AtomSupport;
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
	typedef std::shared_ptr<RPGData> ptr;

	RPGData(const State& seed) :
		_novel(seed.numAtoms()),
		_num_novel(0),
		_current_layer(0),
		_effects()
	{
		// Initially, all domains and deltas are set to contain exactly the values from the seed state
		for (unsigned variable = 0; variable < seed.numAtoms(); ++variable) {
			ObjectIdx value = seed.getValue(variable);
			_effects.insert(std::make_pair(Atom(variable, value),
							std::make_tuple(_current_layer, GroundAction::invalid_action_id, std::make_shared<std::vector<Atom>>())));
		}
	};

	~RPGData() {};

	//! Returns the number of layers of the RPG.
	unsigned getNumLayers() const  {return _current_layer + 1; } // 0-indexed!
	
	//! Returns the current layer index
	unsigned getCurrentLayerIdx() const  {return _current_layer; }

	//! Closes the last RPG layer and opens up a new one
	void advanceLayer() {
		_num_novel= 0;
		_novel = std::vector<std::vector<ObjectIdx>>(_novel.size()); // completely clear the vector of novel atoms
		++_current_layer;
	}

	//! Returns the support for the given atom
	const AtomSupport& getAtomSupport(const Atom& atom) const {
		auto it = _effects.find(atom);
		assert(it != _effects.end());
		return it->second;
	}

	//! Get the number of novel atoms in the last layer of the RPG
	unsigned getNumNovelAtoms() const { return _num_novel; }
	
	const std::vector<std::vector<ObjectIdx>>& getNovelAtoms() const { return _novel; }


	//! Returns true iff the given atom is not already tracked by the RPG. In that case, it returns an insertion hint too.
	std::pair<bool, SupportMap::iterator> getInsertionHint(const Atom& atom) {
		SupportMap::iterator lb = _effects.lower_bound(atom); // @see http://stackoverflow.com/a/101980
		bool keyInMap = lb != _effects.end() && !(_effects.key_comp()(atom, lb->first));
		if (keyInMap) return std::make_pair(false, _effects.end());
		else return std::make_pair(true, lb);
	}

	//! The version with hint assumes that the atom needs to be inserted.
	void add(const Atom& atom, ActionIdx action, Atom::vctrp support, SupportMap::iterator hint) {
		_effects.insert(hint, std::make_pair(atom, std::make_tuple(_current_layer, action, support)));
		_novel[atom.getVariable()].push_back(atom.getValue());
		++_num_novel;
	}
	
	//! Add an atom to the set of newly-reached atoms, only if it is indeed new.
	void add(const Atom& atom, ActionIdx action, Atom::vctrp support) {
		auto hint = getInsertionHint(atom);
		if (!hint.first) return; // Don't insert the atom if it was already tracked by the RPG
		add(atom, action, support, hint.second);
	}

	friend std::ostream& operator<<(std::ostream &os, const RPGData& data) { return data.print(os); }

	//! Prints a representation of the RPG data to the given stream.
	std::ostream& print(std::ostream& os) const {
		os << "Relaxed Planning Graph atoms (" << _effects.size() << "): " << std::endl;
		for (const auto& x:_effects) {
			os << x.first  << " - action #" << std::get<1>(x.second) << " - layer #" << std::get<0>(x.second) << " - support: ";
			printAtoms(std::get<2>(x.second), os);
			os << std::endl;
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
