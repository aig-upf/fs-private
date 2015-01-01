
#pragma once

#include <vector>
#include <ostream>

#include <core_types.hxx>
#include <state.hxx>
#include <actions.hxx>

namespace aptk { namespace core {
	
class Utils {
public:
	typedef std::vector<std::vector<unsigned>*> ValueSet;
	typedef std::vector<unsigned> Point;
	
	template <typename IteratorCallback>
	static void iterateCartesianProduct(
		const ValueSet& values,
		IteratorCallback callback
	) {
		ProcedurePoint tmp;
		_iterateCartesianProduct(values, callback, tmp, 0);
	}

protected:
	template <typename IteratorCallback>
	static void _iterateCartesianProduct(const ValueSet& values, IteratorCallback callback, ProcedurePoint& current, unsigned idx) {
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


} } // namespaces
