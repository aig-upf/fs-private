
#pragma once

#include <vector>
#include <ostream>

#include <problem_info.hxx>

namespace aptk { namespace core {

class Problem;

class Printers {
public:
	static void printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out);
	static void printPlan(const std::vector<int>& plan, const ProblemInfo::cptr& problemInfo, std::ostream& out);
};


} } // namespaces
