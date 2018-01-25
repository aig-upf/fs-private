
#include <numeric>
#include <fs/core/utils/cartesian_iterator.hxx>

namespace fs0 { namespace utils {

cartesian_iterator::cartesian_iterator(std::vector<const std::vector<object_id>*>&& values) :
	_values(std::move(values)),
	_iterators(),
	_element(),
	_ended(_values.empty())
{
	// Initialize the iterator vector and check that all the sets of the cartesian product have at least one element (otherwise the product will be empty)
	_iterators.reserve(_values.size());
	_element.reserve(_values.size());
	for (const auto& domain:_values) {
		if (domain->empty()) {
			_ended = true;
			break;
		}
		auto it = domain->begin();
		_iterators.push_back(it);
		_element.push_back(*it);
	}
}


//! Compute the size of the cartesian product
unsigned long cartesian_iterator::size() const {
	return std::accumulate(_values.begin(), _values.end(), (unsigned long) 1, [](int a, const std::vector<object_id>* b) { return a * b->size(); });
}

//! Advances the iterator at position 'idx' or, if it has reached the end, resets its and tries with the one at the left, recursively.
void cartesian_iterator::advanceIterator(unsigned idx) {
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

void cartesian_iterator::updateElement(unsigned idx) {
	assert(_iterators[idx] != _values[idx]->end());
	_element[idx] = *(_iterators[idx]);
}
	
} } // namespaces
