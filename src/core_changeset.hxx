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

#include <core_types.hxx>
#include <actions.hxx>
#include <fact.hxx>
#include <state.hxx>

#include <map>
#include <cassert>

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
	//! The action that is currently modifying the changeset.
	ActionIdx _activeActionIdx;
	
	/**
	 * A map mapping every atom reached in the RPG layer represented by this changeset to a pair <a, b>, where:
	 *  - `a` is one of the actions that achieves the atom, arbitrarily chosen
	 *  - `b` contains a number of "extra" causes, i.e. atoms that are necessary for the achievement of the first atom
	 *    because they relate to the relevant variables of the effect procedure that achieves it.
	 */
	std::map<Fact, std::pair<ActionIdx, FactSetPtr>> _effects;
	
	/**
	 * A map mapping every action applicable in this layer to the supports of the action, i.e. the atoms that make it applicable.
	 */	
	std::unordered_map<ActionIdx, FactSetPtr> _causes;

	//! We keep a pointer to the previous RPG layer to ensure that we only add novel atoms.
	const RelaxedState* _referenceState;

public:
	typedef std::shared_ptr<Changeset> ptr;
	
	Changeset() : 
		_activeActionIdx(),
		_effects(),
		_causes(),
		_referenceState()
	{};
	
	Changeset(const RelaxedState* referenceState) : 
		_activeActionIdx(),
		_effects(),
		_causes(),
		_referenceState(referenceState)
	{};	
	
	~Changeset() {};
	
	virtual void add(const Fact& fact, const FactSetPtr extraCauses) {
		if (_referenceState && _referenceState->contains(fact)) { // If we have a reference state, we want to ensure that the fact is novel.
			return;
		}
		
		// Note that this might overwrite a previous action - so far, we do not care which action
		// remains as the achiever, as long as there is at least one. This might leave some room for improvement.
		_effects[fact] = std::make_pair(_activeActionIdx, extraCauses);
	}
	
	virtual void add(const Fact& fact) { add(fact, nullptr); }
	
	/**
	 * Returns the achiever action of the given effect, if it is on the changeset,
	 * or CoreAction::INVALID_ACTION otherwise.
	 * Returns also any extra causes for the fact.
	 */
	std::pair<ActionIdx, FactSetPtr> getAchieverAndCauses(const Fact& fact) {
		auto it = _effects.find(fact);
		return (it != _effects.end()) ? it->second : std::make_pair(CoreAction::INVALID_ACTION, nullptr);
	}
	
	FactSetPtr getCauses(const ActionIdx& actionIdx) const { return _causes.at(actionIdx); }
	
	unsigned size() const { return _effects.size(); }
	
	//! Sets the action that will be considered the support for the atoms added in subsequent calls to the "add" method
	void setCurrentAction(const ActionIdx activeActionIdx, FactSetPtr causes) {
		assert(!_causes.count(activeActionIdx));
		_activeActionIdx = activeActionIdx;
		_causes[_activeActionIdx] = causes;
	}	
	
	const std::map<Fact, std::pair<ActionIdx, FactSetPtr>>& getEffects() const {
		return _effects;
	}
	
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
			printFactSet(*(x.second), os);
			os << "}";
		}
		os << std::endl;
		return os;
	}
	
	void printFactSet(const FactSet& fs, std::ostream& os) const {
		for (const auto& fact:fs) {
			os << fact << ", ";
		}
	}
	
	std::ostream& printEffects(std::ostream& os) const {
		os << "Changeset effects: ";
		for (const auto& x:_effects) {
			os << x.first  << " (action #" << x.second.first << "), (extra: ";
			printFactSet(*(x.second.second), os);
			os << "), ";
		}
		os << std::endl;
		return os;
	}	
};


} } // namespaces
