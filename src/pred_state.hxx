//
// deprecated
//


#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cassert>
#include <boost/functional/hash.hpp>

#include <core_types.hxx>
#include <fact.hxx>

namespace aptk { namespace core {

class Problem; class Changeset;

typedef std::set<ObjectIdx> ObjectSet;

class PredicativeState
{
	const static uint8_t VAL_FALSE;
	const static uint8_t VAL_TRUE;
	const static uint8_t VAL_BOTH;
	
	const static ObjectSet VEC_FALSE;
	const static ObjectSet VEC_TRUE ;
	const static ObjectSet VEC_BOTH;
	
protected:
	//! A vector associating symbol IDs to their current extensional value in the state.
	std::vector<uint8_t> _values;

	bool _relaxed; // TODO - Remove and refactor into two different classes.
	
	std::size_t _hash;

public:
	typedef std::shared_ptr<PredicativeState> ptr;
	typedef std::shared_ptr<const PredicativeState> cptr;

	virtual ~PredicativeState() {}
	
	//! Construct a state specifying all the predicates and functions - this should indeed be the only way,
	//! although ATM we are forced to leave the no-arguments constructor in order to be able
	//! to set a Problem's initial state after construction
	//! TODO - We might want to perform type checking here against the predicate and function signatures.
	//! TODO - We might also want to ensure here that all symbol extensions have been defined. This won't be expensive, 
	//! as it will be done only when we create the initial state.
	PredicativeState(unsigned numFacts, const FactVector& facts) :
		_values(numFacts), _relaxed(false)
	{
		for (const auto& fact:facts) { // Insert all the elements of the vector
			assert(fact._value == 0 || fact._value == 1);
			if (fact._value) {
				_values[fact._variable] = VAL_TRUE;
			}
		}
		updateHash();
	};
	
	/**
	 * Copy constructor
	 */
	PredicativeState(const PredicativeState& state) :
		_values(state._values), _relaxed(state._relaxed), _hash(state._hash)
	{}
	
	//! A constructor that receives a changeset and construct a state that is equal to the received
	//! state plus the changes in the changeset.
	PredicativeState(const PredicativeState& state, const Changeset& changeset) :
		_values(state._values), _relaxed(state._relaxed), _hash(state._hash) {
		applyChangeset(changeset);
	}
	
	// TODO - This is probably not exception-safe
	PredicativeState& operator=(const PredicativeState &rhs) {
		if (this == &rhs) return *this;
		_values = rhs._values;
		_hash = rhs._hash;
		_relaxed = rhs._relaxed;
		return *this;
	}
	
	bool operator==(const PredicativeState &rhs) const {
		return _hash == rhs._hash && _relaxed == rhs._relaxed && _values == rhs._values;
	}
	
	bool operator!=(const PredicativeState &rhs) const { return !(this->operator==(rhs));}
	
	void set(const Fact& fact) {
		assert(fact._value == 0 || fact._value == 1);
		uint8_t newVal = fact._value ? VAL_TRUE : VAL_FALSE;
		uint8_t& current = _values.at(fact._variable);
		if (!_relaxed) { // If this is NOT a relaxed extension, we only allow one value in the objectSet.
			current = newVal;
		} else {
			if (current != newVal) {
				current = VAL_BOTH;
			}
		}
	}
	
	bool contains(const Fact& fact) const {
		assert(fact._value == 0 || fact._value == 1);
		return getValues(fact._variable).count(fact._value) > 0;
	}
	
	const ObjectSet& getValues(const StateVariableIdx& variable) const {
		uint8_t current = _values.at(variable);
		if (current == VAL_TRUE) return VEC_TRUE;
		if (current == VAL_FALSE) return VEC_FALSE;
		return VEC_BOTH;
	}	
	
protected:
	//! Applies the given changeset into the current state.
	void applyChangeset(const Changeset& changeset);
	
	void updateHash() { _hash = computeHash(); }
	
	std::size_t computeHash() const { return boost::hash_range(_values.begin(), _values.end()); };
	
public:
	friend std::ostream& operator<<(std::ostream &os, const PredicativeState&  state) { return state.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const;
	std::ostream& printAll(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const Problem& problem) const;
	
	std::size_t hash() const { return _hash; }
	
	//! Required by Boost.Functional/Hash - Currently we don't need it, if we needed it, it might be better to return _hash directly,
	// as long as there is no circular dependency.
	// friend std::size_t hash_value(PredicativeState const& s) { return s.computeHash(); }
	
	//! Relax a given state.
	static PredicativeState::ptr relax(const PredicativeState& s) {
		PredicativeState* s1 = new PredicativeState(s);
		s1->_relaxed = true;
		s1->updateHash();
		return PredicativeState::ptr(s1);
	}	
	
};

typedef PredicativeState RelaxedPredicativeState;

} } // namespaces

