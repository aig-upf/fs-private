
#pragma once

#include <vector>
#include <ostream>

#include <core_problem.hxx>
#include <problem_info.hxx>

namespace aptk { namespace core {
	
class Printers {
public:
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
};


} } // namespaces
