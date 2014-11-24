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

#ifndef __APTK_CORE_CHANGESET__
#define __APTK_CORE_CHANGESET__

#include <core_types.hxx>
#include <actions.hxx>
#include <fact.hxx>
#include <state.hxx>
#include "justified_action.hxx"

#include <map>
#include <cassert>

namespace aptk { namespace core {

/**
 * A changeset is merely a set of effects.
 */
class Changeset
{
protected:
	//! The action that is currently modifying the changeset.
	BoundActionIdx _activeActionIdx;
	
	/**
	 * A map mapping effects to one of the actions that achieves them, arbitrarily chosen (currently, it just
	 * happens to be the first action we find that achieves the effect).
	 */
	typedef std::map<Fact, std::pair<BoundActionIdx, FactSetPtr>> FactMap;
	FactMap _effects;
	
	//! Maps actions to their causes in a particular layer.
	std::unordered_map<BoundActionIdx, JustifiedAction::ptr> _causes;
	
	RelaxedState::ptr _referenceState;

public:
	Changeset() : 
		_activeActionIdx(),
		_effects(),
		_causes(),
		_referenceState()
	{};
	
	Changeset(const RelaxedState::ptr& referenceState) : 
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
// 		if (_effects.count(fact) == 0) {
// 			_effects.insert({fact, _activeActionIdx});
			_effects[fact] = std::make_pair(_activeActionIdx, extraCauses);
// 		}
	}
	
	virtual void add(const Fact& fact) { add(fact, nullptr); }
	
	/**
	 * Returns the achiever action of the given effect, if it is on the changeset,
	 * or CoreAction::INVALID_ACTION otherwise.
	 * Returns also any extra causes for the fact.
	 */
	std::pair<BoundActionIdx, FactSetPtr> getAchieverAndCauses(const Fact& fact) {
		auto it = _effects.find(fact);
		return (it != _effects.end()) ? it->second : std::make_pair(CoreAction::INVALID_ACTION, nullptr);
	}
	
	const FactSet& getCauses(const BoundActionIdx& actionIdx) const { return _causes.at(actionIdx)->getCauses(); }
	
	std::ostream& printCauses(std::ostream& os) const {
		os << "Changeset causes for:";
		for (const auto& x:_causes) {
			os << " #" << x.first  << ": {";
			printFactSet(x.second->getCauses(), os);
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
	
	unsigned size() const { return _effects.size(); }
	
	void setJustifiedAction(const BoundActionIdx activeActionIdx, JustifiedAction::ptr action) {
		assert(!_causes.count(activeActionIdx));
		_activeActionIdx = activeActionIdx;
		_causes[_activeActionIdx] = action;
	}	
	
	const FactMap& getEffects() const {
		return _effects;
	}
	
	friend std::ostream& operator<<(std::ostream &os, const Changeset&  cs) { return cs.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const {
		printEffects(os);
		printCauses(os);
		return os;
	}
};

typedef std::shared_ptr<Changeset> ChangesetPtr;


} } // namespaces

#endif
