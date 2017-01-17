
#pragma once

// #define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS


#include <fs_types.hxx>
// #include <boost/dynamic_bitset.hpp>

// namespace boost {
//     template <typename B, typename A>
//     std::size_t hash_value(const boost::dynamic_bitset<B, A>& bs) {
//         return boost::hash_value(bs.m_bits);
//     }
// }

namespace fs0 {

class Problem;
class Atom;

class State {
protected:
	//! A vector mapping state variable (implicit) ids to their value in the current state.
	std::vector<bool> _values;
// 	boost::dynamic_bitset<> _values;

	std::size_t _hash;

public:
	~State() {}
	
	//! Construct a state specifying the values of all state variables
	//! Note that it is not necessarily the case that numAtoms == facts.size(); since the initial values of
	//! some (Boolean) state variables is often left unspecified and understood to be false.
	State(unsigned numAtoms, const std::vector<Atom>& facts);
	
	//! A constructor that receives a number of atoms and constructs a state that is equal to the received
	//! state plus the new atoms. Note that we do not check that there are no contradictory atoms.
	State(const State& state, const std::vector<Atom>& atoms);
	
	//! Default copy constructors and assignment operators - if ever need a custom version, check the git history!
	// https://bitbucket.org/gfrances/fs0/src/28ce4119f27a537d8f7628c6ca0487d03d5ed0b1/src/state.hxx?at=gecode_integration
	State(const State&) = default;
	State(State&&) = default;
	State& operator=(const State&) = default;
	State& operator=(State&&) = default;

	// Check the hash first for performance.
	bool operator==(const State &rhs) const { return _hash == rhs._hash && _values == rhs._values; }
	bool operator!=(const State &rhs) const { return !(this->operator==(rhs));}
	
	
	bool contains(const Atom& atom) const;
	
	ObjectIdx getValue(const VariableIdx& variable) const;
	
	unsigned numAtoms() const { return _values.size(); }
	
	//! "Applies" the given atoms into the current state.
	void accumulate(const std::vector<Atom>& atoms);

protected:
	void set(const Atom& atom);
	
	void updateHash() { _hash = computeHash(); }
	
	std::size_t computeHash() const;
	
public:
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const State&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	std::size_t hash() const { return _hash; }
};

} // namespaces

