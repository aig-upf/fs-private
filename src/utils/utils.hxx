
#pragma once

#include <vector>
#include <ostream>

#include <fs0_types.hxx>
#include <state.hxx>

namespace fs0 {
	
class Utils {
public:
	typedef std::vector<std::vector<unsigned>*> ValueSet;
	typedef std::vector<unsigned> Point;
	
	template <typename IteratorCallback>
	static void iterateCartesianProduct(
		const ValueSet& values,
		IteratorCallback callback
	) {
		ObjectIdxVector tmp;
		_iterateCartesianProduct(values, callback, tmp, 0);
	}
	
	template <typename T>
	static std::vector<T> merge(const std::vector<T>& vector1, const std::vector<T>& vector2) {
		std::vector<T> result(vector1);
		result.insert(result.end(), vector2.begin(), vector2.end());
		return result;
	}
	
	//! Flattens a container of containers
	template <typename T>
	static std::vector<T> flatten(const std::vector<std::vector<T>>& vector) {
		std::vector<T> result;
		for(const std::vector<T>& sub:vector) {
			for(T var:sub) {
				result.push_back(var);
			}
		}
		return result;
	}	
	
	//! "Uniquifies" the given vector.
	template <typename T>
	static std::vector<T> unique(const std::vector<T>& vector) {
		boost::container::flat_set<T> unique(vector.cbegin(), vector.cend());
		return std::vector<T>(unique.cbegin(), unique.cend());
	}	

protected:
	template <typename IteratorCallback>
	static void _iterateCartesianProduct(const ValueSet& values, IteratorCallback callback, ObjectIdxVector& current, unsigned idx) {
		// base case: we have generated a full element of the cartesian product
		if (idx == values.size() - 1) {
			callback(current);
			return;
		}
		
		// Recursive case: 
		for(auto elem:*(values[idx])) {
			current.push_back(elem); 
			cartesianProduct(values, callback, current, idx+1);
			current.pop_back();
		}
	}
	
};


} // namespaces
