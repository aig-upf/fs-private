
#pragma once

#include <vector>

namespace lapkt {

// struct DefaultPrinter {
// 	std::ostream& operator<<(std::ostream &os, const MultivaluedState&  state) { return state.print(os); }
// };


namespace printers {
	template <typename _T>
	std::ostream& print(std::ostream &os, const _T&  o) { return os << "Undefined Printer"; }
}

template <typename _ValueT = int> //, typename PrinterT = DefaultPrinter>
class MultivaluedState {
public:
	using ValueT = _ValueT;
	
	//! Initialize all values to 0
	MultivaluedState(unsigned size) :
		_values(size, 0), _hash(computeHash())
	{}
	
	//! Initialize all values from a given vector
	MultivaluedState(const std::vector<ValueT>& values) :
		_values(values), _hash(computeHash())
	{}
	
	//! Default destructor, copy and move operators
	~MultivaluedState() = default;
	MultivaluedState(const MultivaluedState&) = default;
	MultivaluedState(MultivaluedState&&) = default;
	MultivaluedState& operator=(const MultivaluedState&) = default;
	MultivaluedState& operator=(MultivaluedState&&) = default;

	//! Equality operators
	bool operator==(const MultivaluedState &rhs) const { return _hash == rhs._hash && _values == rhs._values; } // Check the hash first for performance.
	bool operator!=(const MultivaluedState &rhs) const { return !(this->operator==(rhs));}
	
	
	//! Return the value of a given variable
	ValueT value(unsigned index) const { return _values.at(index); }
	
	//! Return the whole set of values of the state variables
	const std::vector<ValueT>& values() const { return _values; }
	
	//! Checks whether a certain variable has the given value
	bool check(unsigned index, ValueT val) const { return value(index) == val; }

	//! Return the number of state variables
	unsigned size() const { return _values.size(); }
	
	//! Update the value of a single variable.
	//! This operation has linear cost, as it involves updating the internal hash of the state.
	void set(unsigned variable, ValueT val) {
		_values.at(variable) = val;
		_hash = computeHash();
	}
	
	//! Update the value of a set of variables.
	//! This operation has linear cost, as it involves updating the internal hash of the state.
	void set(const std::vector<std::pair<unsigned, ValueT>>& atoms) {
		for (const std::pair<unsigned, ValueT>& atom:atoms) { 
			_values.at(atom.first) = atom.second;
		}
		_hash = computeHash(); // Update the hash once after all values have been updated
	}
	
	//! Return the hash of the state
	std::size_t hash() const { return _hash; }

protected:
	//! A helper to compute the hash of the state.
	//! This operation has linear cost in the number of variables
	std::size_t computeHash() const;
	
	//! A vector mapping state variable (implicit) ids to their value in the current state.
	std::vector<ValueT> _values;

	//! The hash of the state
	std::size_t _hash;

public:
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const MultivaluedState&  state) { return printers::print(os, state); }
	// friend std::ostream& operator<<(std::ostream &os, const MultivaluedState&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
};

} // namespaces
