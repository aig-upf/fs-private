
#pragma once

#include <fs_types.hxx>
#include <iostream>

namespace fs0 {

//! A (possibly partial) binding of quantified variables / action parameters to actual values
class Binding {
protected:
	std::vector<bool> _set;
	ValueTuple _values;
	
public:
	//! Construct an empty binding
	Binding() {}
	
	//! Construct a binding of given size with all variables unset
	Binding(std::size_t size) : _set(size, false), _values(size) {}
	
	//! Construct a complete binding from a given vector with all variables set
	Binding(const ValueTuple& values) : Binding(ValueTuple(values)) {}
	Binding(ValueTuple&& values) : _set(values.size(), true), _values(std::move(values)) {}
	
	Binding(const ValueTuple& values, const std::vector<bool>& set) : _set(set), _values(values) {}
	Binding(ValueTuple&& values, std::vector<bool>&& set) : _set(std::move(set)), _values(std::move(values)) {}
	
	//! Default copy constructor
	Binding(const Binding& other) = default;
	Binding(Binding&& other) = default;
	Binding& operator=(const Binding& rhs) = default;
	Binding& operator=(Binding&& rhs) = default;
	
	//! Merge in-place the current binding with the given one. If the two bindings overlap, i.e. both bind
	//! one same variable, throws an exception
	void merge_with(const Binding& other) {
		assert(size() == other.size());
		for (unsigned i = 0; i < _values.size(); ++i) {
			if (other.binds(i)) {
				if (this->binds(i)) throw std::runtime_error("Attempting to merge to overlapping bindings");
				set(i, other.value(i));
			}
		}
	}
	
	bool is_complete() const {
		for (bool b:_set) {
			if (!b) return false;
		}
		return true;
	}
	
	//! (Only) if the binding is full, we can obtain the raw vector of integer values 
	const ValueTuple& get_full_binding() const {
		if (!is_complete()) throw std::runtime_error("Attempted to obtain a full binding from a partial binding");
		return _values;
	}
	
	
	//! Returns true iff the current binding contains a binding for the given variable
	bool binds(unsigned variable) const { return variable < _set.size() && _set.at(variable); }
	
	ObjectIdx value(unsigned variable) const {
		assert(binds(variable));
		return _values.at(variable);
	}
	
	void set(unsigned variable, ObjectIdx value) {
		_values[variable] = value;
		_set[variable] = true;
	}
	
	std::size_t size() const { return _values.size(); }

	friend bool operator==(const Binding&, const Binding&);

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Binding& o) { return o.print(os); }
	
	std::ostream& print(std::ostream& os) const {
		os << "{";
		for (unsigned i = 0; i < _values.size(); ++i) {
			if (!binds(i)) continue;
			os << i << ": " << value(i) << ", ";
		}
		os << "}";
		return os;
	}
};

//! Comparison operators
inline bool operator==(const Binding& lhs, const Binding& rhs){ return lhs._set == rhs._set && lhs._values == rhs._values; }
inline bool operator!=(const Binding& lhs, const Binding& rhs){ return !operator==(lhs, rhs); }


} // namespaces
