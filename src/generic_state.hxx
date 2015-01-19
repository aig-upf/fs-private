
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>
#include <boost/functional/hash.hpp>

#include <fs0_types.hxx>
#include <fact.hxx>

namespace fs0 {

class Problem;

class GenericState
{
protected:
	//! A vector associating symbol IDs to their current extensional value in the state.
	std::vector<ObjectIdx> _values;

	std::size_t _hash;

public:
	friend class RelaxedGenericState;
	
	typedef std::shared_ptr<GenericState> ptr;
	typedef std::shared_ptr<const GenericState> cptr;

	virtual ~GenericState() {}
	
	//! Construct a state specifying all the predicates and functions - this should indeed be the only way,
	//! although ATM we are forced to leave the no-arguments constructor in order to be able
	//! to set a Problem's initial state after construction
	//! TODO - We might want to perform type checking here against the predicate and function signatures.
	//! TODO - We might also want to ensure here that all symbol extensions have been defined. This won't be expensive, 
	//! as it will be done only when we create the initial state.
	GenericState(unsigned numFacts, const Fact::vctr& facts) :
		_values(numFacts)
	{
		// Note that those facts not explicitly set in the initial state will be initialized to 0, i.e. "false", which is convenient to us.
		// assert(numFacts == facts.size());
		for (const auto& fact:facts) { // Insert all the elements of the vector
			set(fact);
		}
		updateHash();
	};
	
	
	//! Copy constructor
	GenericState(const GenericState& state) :
		_values(state._values), _hash(state._hash)
	{}
	
	//! A constructor that receives a number of atoms and constructs a state that is equal to the received
	//! state plus the new atoms.
	GenericState(const GenericState& state, const FactSet& atoms) :
		_values(state._values), _hash(state._hash) {
		accumulate(atoms);
	}
	
	//! Assignment operator
	// TODO - This is probably not exception-safe
	GenericState& operator=(const GenericState &rhs) {
		if (this == &rhs) return *this;
		_values = rhs._values;
		_hash = rhs._hash;
		return *this;
	}
	
	bool operator==(const GenericState &rhs) const {
		return _hash == rhs._hash && _values == rhs._values; // Check the hash first for performance.
	}
	
	bool operator!=(const GenericState &rhs) const { return !(this->operator==(rhs));}
	
	void set(const Fact& fact) {
		_values.at(fact._variable) = fact._value;
	}
	
	bool contains(const Fact& fact) const {
		return getValue(fact._variable) == fact._value;
	}
	
	ObjectIdx getValue(const VariableIdx& variable) const {
		return _values.at(variable);
	}
	
protected:
	//! "Applies" the given atoms into the current state.
	void accumulate(const FactSet& atoms);
	
	void updateHash() { _hash = computeHash(); }
	
	std::size_t computeHash() const { return boost::hash_range(_values.begin(), _values.end()); };
	
public:
	friend std::ostream& operator<<(std::ostream &os, const GenericState&  state) { return state.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const;
	std::ostream& printAll(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const Problem& problem) const;
	
	std::size_t hash() const { return _hash; }
	
	//! Required by Boost.Functional/Hash - Currently we don't need it, if we needed it, it might be better to return _hash directly,
	// as long as there is no circular dependency.
	// friend std::size_t hash_value(PredicativeState const& s) { return s.computeHash(); }
};

} // namespaces
