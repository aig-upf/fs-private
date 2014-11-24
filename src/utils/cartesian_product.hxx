
#pragma once

#include <vector>
#include <ostream>
#include <cassert>

namespace aptk { namespace core {

	class CartesianProductIterator {
	public:
		typedef std::vector<std::vector<int>> ValueSet;
		typedef std::vector<int> Point;
		
	protected:
		const ValueSet& _values;
		std::vector<unsigned> _currentIndexes;
		bool _ended;
	
	public:
		CartesianProductIterator(const ValueSet& values) :
			_values(values),
			_currentIndexes(values.size()),  // All indexes get implicitly initialized to 0
			_ended(false)
		{
			checkInitialndexValidity();
		}
		
		//! Check that all the sets of the cartesian product have at least one element - otherwise the product will be empty
		void checkInitialndexValidity() {
			_ended = _ended || _currentIndexes.size() == 0;
			for (unsigned i = 0; i < _currentIndexes.size() && !_ended; ++i) {
				_ended = _ended || _values[i].size() == 0;
			}
		}
		
		void incrementIndex(unsigned idx) {
			assert(idx < _currentIndexes.size());
			if (_currentIndexes[idx] < _values[idx].size() - 1) {
				_currentIndexes[idx]++;
			} else {
				if (idx == 0) { // Base case: We're done with all the elements in the cartesian product.
					_ended = true;
					return;
				}
				
				// otherwise: reset the current idx to zero and try incrementing the previous one.
				_currentIndexes[idx] = 0;
				incrementIndex(idx-1);
			}
		}

	public:
		Point operator*() const {
			// Build a point with the current indexes.
			Point p(_currentIndexes.size());
			for (unsigned i = 0; i < _currentIndexes.size(); ++i) {
				p[i] = _values[i][_currentIndexes[i]];
			}
			return p;
		}
		
		const CartesianProductIterator& operator++() {
			incrementIndex(_currentIndexes.size()-1);
			return *this;
		}
		const CartesianProductIterator operator++(int) {CartesianProductIterator tmp(*this); operator++(); return tmp;}
		
		bool ended() const { return _ended; }

	};
	


} } // namespaces
