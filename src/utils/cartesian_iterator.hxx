
#pragma once

#include <vector>
#include <ostream>
#include <cassert>

#include <fs_types.hxx>

namespace fs0 { namespace utils {

//! Returns an iterable container 
class cartesian_iterator {
protected:
	const std::vector<const std::vector<ObjectIdx>*> _values;
	
	std::vector<std::vector<ObjectIdx>::const_iterator> _iterators;
	
	std::vector<ObjectIdx> _element; // The current element

	bool _ended;

public:
	cartesian_iterator(std::vector<const std::vector<ObjectIdx>*>&& values);

	cartesian_iterator(const cartesian_iterator& o) = default;
	
	//! Compute the size of the cartesian product
	unsigned long size() const;
	
	//! Advances the iterator at position 'idx' or, if it has reached the end, resets its and tries with the one at the left, recursively.
	void advanceIterator(unsigned idx);
	
	void updateElement(unsigned idx);

	const std::vector<ObjectIdx>& operator*() const { return _element; }
	
	const cartesian_iterator& operator++() {
		advanceIterator(_iterators.size()-1);
		return *this;
	}
	const cartesian_iterator operator++(int) {cartesian_iterator tmp(*this); operator++(); return tmp;}
	
	bool ended() const { return _ended; }
};


} } // namespaces
