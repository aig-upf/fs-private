/*
Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <unordered_map>
#include <map>
#include <cassert>
#include <core_types.hxx>
#include <actions.hxx>
#include <fact.hxx>
#include <state.hxx>


namespace aptk { namespace core {

/**
 * A changeset contains book-keeping information concerning the actions that support
 * the achievement of atoms in the Relaxed Planning Graph. This currently includes both
 * the atoms that make an action applicable (in a certain RPG layer) and the "extra"
 * atoms that make a particular effect reachable, i.e. those related to the relevant
 * variables of the effect procedure that achieves the effect.
 */
class Changeset
{
protected:
	
	const GenericState& _seed;
	
	//! The action that is currently modifying the changeset.
	ActionIdx _activeActionIdx;
	
	//! The number of non-seed support atoms for the active action.
	int _activeActionNonSeedSupportAtoms;
	
	/**
	 * A map mapping every atom reached in the RPG layer represented by this changeset to a pair <a, b, c>, where:
	 *  - `a` is one of the actions that achieves the atom, arbitrarily chosen
	 *  - `b` contains a number of "extra" causes, i.e. atoms that are necessary for the achievement of the first atom
	 *    because they relate to the relevant variables of the effect procedure that achieves it.
	 * - `c` is the total number of non-seed supporters of the atom, including the "extra" supporters.
	 */
	typedef std::tuple<ActionIdx, Fact::vctr, unsigned> FactData;
	typedef std::map<Fact, FactData> EffectMap;
	EffectMap _effects;
	
	/**
	 * A map mapping every action applicable in this layer to the supports of the action, i.e. the atoms that make it applicable.
	 */	
	std::unordered_map<ActionIdx, Fact::vctr> _causes;

	//! We keep a pointer to the previous RPG layer to ensure that we only add novel atoms.
	const RelaxedState& _referenceState;
	
	const static FactData INVALID_ACHIEVER;

public:
	typedef std::shared_ptr<Changeset> ptr;
	typedef std::vector<Changeset::ptr> vptr;
	
	Changeset(const State& seed, const RelaxedState& referenceState) : 
		_seed(seed),
		_activeActionIdx(),
		_activeActionNonSeedSupportAtoms(-1),
		_effects(),
		_causes(),
		_referenceState(referenceState)
	{};	
	
	~Changeset() {};
	
	virtual void add(const Fact& fact, const FactSetPtr extraCauses);
	virtual void add(const Fact& fact) { add(fact, nullptr); }
	
	/**
	 * Returns the achiever action of the given effect, if it is on the changeset,
	 * or CoreAction::INVALID_ACTION otherwise.
	 * Returns also any extra causes for the fact.
	 */
	const FactData& getAchieverAndCauses(const Fact& fact) {
		auto it = _effects.find(fact);
		return (it == _effects.end()) ? INVALID_ACHIEVER : it->second;
	}
	
	Fact::vctr const * getCauses(const ActionIdx& actionIdx) const { return &(_causes.at(actionIdx)); }
	
	unsigned size() const { return _effects.size(); }
	
	//! Sets the action that will be considered the support for the atoms added in subsequent calls to the "add" method
	void setCurrentAction(const ActionIdx activeActionIdx, Fact::vctr&& causes) {
		assert(!_causes.count(activeActionIdx));
		_activeActionIdx = activeActionIdx;
		_activeActionNonSeedSupportAtoms = causes.size();
		_causes.insert(std::make_pair(_activeActionIdx, causes));
	}
	
	const EffectMap& getEffects() const { return _effects; }
	
	friend std::ostream& operator<<(std::ostream &os, const Changeset&  cs) { return cs.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const {
		printEffects(os);
		printCauses(os);
		return os;
	}
	
	std::ostream& printCauses(std::ostream& os) const {
		os << "Changeset causes for:";
		for (const auto& x:_causes) {
			os << " #" << x.first  << ": {";
			printFactSet(x.second, os);
			os << "}";
		}
		os << std::endl;
		return os;
	}
	
	void printFactSet(const Fact::vctr& vector, std::ostream& os) const {
		for (const auto& fact:vector) {
			os << fact << ", ";
		}
	}
	
	std::ostream& printEffects(std::ostream& os) const {
		os << "Changeset effects: ";
		for (const auto& x:_effects) {
			os << x.first  << " (action #" << std::get<0>(x.second) << "), (extra: ";
			printFactSet(std::get<1>(x.second), os);
			os << "), ";
		}
		os << std::endl;
		return os;
	}	
};


} } // namespaces
