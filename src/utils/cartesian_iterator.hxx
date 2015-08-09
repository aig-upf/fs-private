
#pragma once

#include <vector>
#include <ostream>
#include <cassert>

#include <fs0_types.hxx>

namespace fs0 { namespace utils {

//! Returns an iterable container 
class cartesian_iterator {
protected:
	const std::vector<const ObjectIdxVector*> _values;
	
	std::vector<ObjectIdxVector::const_iterator> _iterators;
	
	std::vector<ObjectIdx> _element; // The current element

	bool _ended;

public:
	cartesian_iterator(std::vector<const ObjectIdxVector*>&& values) :
		_values(values),
		_iterators(),
		_element(),
		_ended(_values.empty())
	{
		// Initialize the iterator vector and check that all the sets of the cartesian product have at least one element (otherwise the product will be empty)
		_iterators.reserve(_values.size());
		_element.reserve(_values.size());
		for (const auto& domain:_values) {
			if (domain->size() == 0) {
				_ended = true;
				break;
			}
			auto it = domain->begin();
			_iterators.push_back(it);
			_element.push_back(*it);
		}
	}

	cartesian_iterator( const cartesian_iterator& o ) 
	:	_values ( o._values ),
		_iterators( o._iterators ),
		_element( o._element ),
		_ended( o._ended )
	{}
	
	//! Advances the iterator at position 'idx' or, if it has reached the end, resets its and tries with the one at the left, recursively.
	void advanceIterator(unsigned idx) {
		assert(idx < _iterators.size());
		if (++_iterators[idx] != _values[idx]->end()) {
			updateElement(idx);
		} else {
			if (idx == 0) { // Base case: We're done with all the elements in the cartesian product.
				_ended = true;
				return;
			}
			
			// otherwise: reset the current idx to zero and try incrementing the previous one.
			_iterators[idx] = _values[idx]->begin();
			updateElement(idx);
			advanceIterator(idx-1);
		}
	}
	
	inline void updateElement(unsigned idx) {
		assert(_iterators[idx] != _values[idx]->end());
		_element[idx] = *(_iterators[idx]);
	}

public:
	const std::vector<ObjectIdx>& operator*() const { return _element; }
	
	const cartesian_iterator& operator++() {
		advanceIterator(_iterators.size()-1);
		return *this;
	}
	const cartesian_iterator operator++(int) {cartesian_iterator tmp(*this); operator++(); return tmp;}
	
	bool ended() const { return _ended; }
};


} } // namespaces
