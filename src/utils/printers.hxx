
#pragma once

#include <vector>
#include <ostream>

#include <problem_info.hxx>
#include <actions.hxx>

namespace fs0 {

class Problem;

class Printers {
public:
	static void printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out);
	static void printPlan(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);
};


} // namespaces
