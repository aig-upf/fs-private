
#pragma once

#include <algorithm>
#include <fs_types.hxx>

namespace fs0 {

//! A (possibly partial) binding of quantified variables / action parameters to actual values
class Binding {
protected:
	std::vector<bool> _set;
	ValueTuple _values;
	
public:
	//! A const empty binding, to be used whenever needed.
	const static Binding EMPTY_BINDING;
	
	//! Construct an empty binding
	Binding() = default;
	
	//! Construct a binding of given size, all variables unset
	Binding(std::size_t size) : _set(size, false), _values(size) {}
	
	//! Construct a complete binding from a given vector with all variables set to the values in the vector
	Binding(const ValueTuple& values) : Binding(ValueTuple(values)) {}
	Binding(ValueTuple&& values) : _set(values.size(), true), _values(std::move(values)) {}
	
	//! Construct a complete binding from a given vector, but only taking into account those variables specified in 'set'
	Binding(const ValueTuple& values, const std::vector<bool>& set) : Binding(ValueTuple(values), std::vector<bool>(set)) {}
	Binding(ValueTuple&& values, std::vector<bool>&& set) : _set(std::move(set)), _values(std::move(values)) {}
	
	//! Default copy constructor
	Binding(const Binding&) = default;
	Binding(Binding&&) = default;
	Binding& operator=(const Binding&) = default;
	Binding& operator=(Binding&&) = default;
	
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
	
	//! Returns true iff all variables in the binding are complete
	bool is_complete() const {
		return std::find(_set.cbegin(), _set.cend(), false) == _set.cend();
	}
	
	//! (Only) if the binding is full, we can obtain the raw vector of integer values 
	const ValueTuple& get_full_binding() const {
		if (!is_complete()) throw std::runtime_error("Attempted to obtain a full binding from a partial binding");
		return _values;
	}
	
	//! Returns true iff the current binding contains a binding for the given variable
	bool binds(unsigned variable) const { return variable < _set.size() && _set.at(variable); }
	
	object_id value(unsigned variable) const {
		if (!binds(variable)) throw std::runtime_error("Attempted to access unset binding position");
		return _values.at(variable);
	}
	
	//! Sets the given position of the binding to the given value; if necessary, resizes the binding.
	void set(unsigned variable, const object_id& value) {
		unsigned min_size = variable + 1;
		if (min_size > _set.size()) {
			_values.resize(min_size);
			_set.resize(min_size, false);
		}
		_values[variable] = value;
		_set[variable] = true;
	}
	
	std::size_t size() const { return _values.size(); }

	friend bool operator==(const Binding&, const Binding&);

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Binding& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
};

//! Comparison operators
inline bool operator==(const Binding& lhs, const Binding& rhs){ return lhs._set == rhs._set && lhs._values == rhs._values; }
inline bool operator!=(const Binding& lhs, const Binding& rhs){ return !operator==(lhs, rhs); }


} // namespaces
