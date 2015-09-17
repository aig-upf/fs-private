
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>
#include <boost/functional/hash.hpp>

#include <fs0_types.hxx>
#include <atom.hxx>

namespace fs0 {

class Problem;

class State {
protected:
	//! A vector mapping state variable (implicit) ids to their value in the current state.
	std::vector<ObjectIdx> _values;

	std::size_t _hash;

public:
	friend class RelaxedState;
	
	typedef std::shared_ptr<State> ptr;
	typedef std::shared_ptr<const State> cptr;

	~State() {}
	
	//! Construct a state specifying the values of all state variables
	//! Note that it is not necessarily the case that numAtoms == facts.size(); since the initial values of
	//! some (Boolean) state variables is often left unspecified and understood to be false.
	//! TODO - We might want to perform type checking here against the predicate and function signatures.
	//! TODO - We might also want to ensure here that all symbol extensions have been defined. This won't be expensive, 
	//! as it will be done only when we create the initial state.
	State(unsigned numAtoms, const std::vector<Atom>& facts) :
		_values(numAtoms)
	{
		// Note that those facts not explicitly set in the initial state will be initialized to 0, i.e. "false", which is convenient to us.
		for (const auto& fact:facts) { // Insert all the elements of the vector
			set(fact);
		}
		updateHash();
	};
	
	//! A constructor that receives a number of atoms and constructs a state that is equal to the received
	//! state plus the new atoms. Note that we do not check that there are no contradictory atoms.
	State(const State& state, const std::vector<Atom>& atoms) :
		State(state) {
		accumulate(atoms);
	}
	
	//! Default copy constructors and assignment operators - if ever need a custom version, check the git history!
	// https://bitbucket.org/gfrances/fs0/src/28ce4119f27a537d8f7628c6ca0487d03d5ed0b1/src/state.hxx?at=gecode_integration
	State(const State& state) = default;
	State( State&& state ) = default;
	State& operator=(const State &state) = default;
	State& operator=(State&& state) = default;

	// Check the hash first for performance.
	bool operator==(const State &rhs) const { return _hash == rhs._hash && _values == rhs._values; }
	bool operator!=(const State &rhs) const { return !(this->operator==(rhs));}
	
	void set(const Atom& atom) {
		_values.at(atom.getVariable()) = atom.getValue();
	}
	
	bool contains(const Atom& atom) const {
		return getValue(atom.getVariable()) == atom.getValue();
	}
	
	ObjectIdx getValue(const VariableIdx& variable) const {
		return _values.at(variable);
	}

	unsigned numAtoms() const { return _values.size(); }
	
protected:
	//! "Applies" the given atoms into the current state.
	void accumulate(const std::vector<Atom>& atoms);
	
	void updateHash() { _hash = computeHash(); }
	
	std::size_t computeHash() const { return boost::hash_range(_values.begin(), _values.end()); };
	
public:
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const State&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	std::size_t hash() const { return _hash; }
};

} // namespaces
