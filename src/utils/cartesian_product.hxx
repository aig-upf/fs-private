
#pragma once

#include <vector>
#include <ostream>
#include <cassert>

#include <fs0_types.hxx>

namespace fs0 {

class CartesianProductIterator {
protected:
	const DomainVector& _values;
	std::vector<Domain::const_iterator> _iterators;
	bool _ended;

public:
	CartesianProductIterator(const DomainVector& values) :
		_values(values),
		_iterators(),
		_ended(values.size() == 0)
	{
		// Initialize the iterator vector and check that all the sets of the cartesian product have at least one element
		// (otherwise the product will be empty)
		for (const DomainPtr& domain:values) {
			if (domain->size() == 0) {
				_ended = true;
				break;
			}
			_iterators.push_back(domain->begin());
		}
	}
	
	//! Advances the iterator at position 'idx' or, if it has reached the end, resets its and tries with the one at the left, recursively.
	void advanceIterator(unsigned idx) {
		assert(idx < _iterators.size());
		if (_iterators[idx] != _values[idx]->end()) {
			++_iterators[idx];
		} else {
			if (idx == 0) { // Base case: We're done with all the elements in the cartesian product.
				_ended = true;
				return;
			}
			
			// otherwise: reset the current idx to zero and try incrementing the previous one.
			_iterators[idx] = _values[idx]->begin();
			advanceIterator(idx-1);
		}
	}

public:
	std::vector<ObjectIdx> operator*() const {
		// Build a point with the current indexes.
		std::vector<ObjectIdx> element;
		element.reserve(_iterators.size());
		for (const auto& position:_iterators) {
			element.push_back(*position); // Dereference the iterator.
		}
		return element;
	}
	
	const CartesianProductIterator& operator++() {
		advanceIterator(_iterators.size()-1);
		return *this;
	}
	const CartesianProductIterator operator++(int) {CartesianProductIterator tmp(*this); operator++(); return tmp;}
	
	bool ended() const { return _ended; }

};



} // namespaces
