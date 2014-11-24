
#pragma once

#include <vector>
#include <ostream>

#include <core_problem.hxx>
#include <core_types.hxx>
#include <problem_info.hxx>


namespace aptk { namespace core {
	
class Utils {
public:
	typedef std::vector<std::vector<unsigned>*> ValueSet;
	typedef std::vector<unsigned> Point;
	
	static void printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out) {
		auto problemInfo = problem.getProblemInfo();
		for (int action:plan) {
			out << problemInfo->getActionName(action) << std::endl;
		}
	}
	
	static void printPlan(const std::vector<int>& plan, const ProblemInfo::cptr& problemInfo, std::ostream& out) {
		for (int action:plan) {
			out << problemInfo->getActionName(action) << std::endl;
		}
	}
	
	/**
	* 
	* @params all: The final result
	*/
// 	static std::vector<ObjectIdx> cartesianProduct(
// 		const ValueSet& values,
// 		std::vector<ProcedurePoint>& all,
// 		ProcedurePoint& current,
// 		unsigned idx)
// 	{
// 		if (idx == values.size() - 1) {
// 			all.push_back(current);
// 			return;
// 		}
// 		
// 		for(auto elem:*(values[idx])) {
// 			current.push_back(elem);
// 			cartesianProduct(values, all, current, idx+1);
// 			current.pop_back();
// 		}
// 	}
	
	
// 	typedef std::function<bool (const ProcedurePoint&)> IteratorCallback;
	
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
