
#pragma once

#include <fs_types.hxx>
//#include <utils/bitsets.hxx>
#include <problem_info.hxx>

namespace fs0 {

class Problem;
class Atom;
class StateAtomIndexer;
class State;

class StateAtomIndexer {
public:
	using IndexElemT = std::pair<bool, unsigned>;
	using IndexT = std::vector<IndexElemT>;
	
protected:
	//! Assume _index[v] = (b,i). This means that the value of the state variable v
	//! is stored in the i-th position of the vector of bools (if b is true) or the vector
	//! of ints (if b is false)
	const IndexT _index;

	std::size_t                _n_bool;
	std::size_t                _n_int;
    	const ProblemInfo&         _info;

	//! Private constructor
	StateAtomIndexer(IndexT&& index, unsigned n_bool, unsigned n_int, const ProblemInfo& info );

public:
	//! Factory method
	static StateAtomIndexer* create(const ProblemInfo& info);
	
	std::size_t size() const { return _index.size(); }
	
	std::size_t num_bool() const { return _n_bool; }
	std::size_t num_int() const { return _n_int; }

	bool is_fully_binary() const { return _n_int == 0; }
	bool is_fully_multivalued() const { return _n_bool == 0; }
	
	//! Obtain and return the value of the given variable from the given state
	ObjectIdx get(const State& state, VariableIdx variable) const;
	
	//! Set a value into the state
	void set(State& state, const Atom& atom) const;
	void set(State& state, VariableIdx variable, ObjectIdx value) const;
};

class State {
	friend class StateAtomIndexer;
public:
	// using BitsetT = boost::dynamic_bitset<>;
	using BitsetT = std::vector<bool>;
	using IntsetT = std::vector<int>;

protected:
	const StateAtomIndexer& _indexer;
	
	//! A vector mapping state variable (implicit) ids to their value in the current state.
	BitsetT _bool_values;
	IntsetT _int_values;

	std::size_t _hash;

protected:
	//! Construct a state specifying the values of all state variables
	//! Note that it is not necessarily the case that numAtoms == atoms.size(); since the initial values of
	//! some (Boolean) state variables is often left unspecified and understood to be false.
	State(const StateAtomIndexer& index, const std::vector<Atom>& atoms);

public:
	~State() = default;
	
	//! Factory method
	static State* create(const StateAtomIndexer& index, unsigned numAtoms, const std::vector<Atom>& atoms);
	

	//! A constructor that receives a number of atoms and constructs a state that is equal to the received
	//! state plus the new atoms. Note that we do not check that there are no contradictory atoms.
	State(const State& state, const std::vector<Atom>& atoms);

	//! Default copy constructors and assignment operators
	State(const State&) = default;
	State(State&&) = default;
	State& operator=(const State&) = default;
	State& operator=(State&&) = default;

	// Check the hash first for performance.
	bool operator==(const State &rhs) const { return _hash == rhs._hash && _bool_values == rhs._bool_values && _int_values == rhs._int_values; }
	bool operator!=(const State &rhs) const { return !(this->operator==(rhs));}


	bool contains(const Atom& atom) const;

	ObjectIdx getValue(const VariableIdx& variable) const;
    	int       getIntValue( const VariableIdx& variable) const;

	unsigned numAtoms() const { return _bool_values.size() + _int_values.size(); }

	//! "Applies" the given atoms into the current state.
	void accumulate(const std::vector<Atom>& atoms);

	const BitsetT& get_boolean_values() const {
		assert(_indexer.is_fully_binary());
		return _bool_values;
	}
	const IntsetT& get_int_values() const {
		assert(_indexer.is_fully_multivalued());
		return _int_values;
	}
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

