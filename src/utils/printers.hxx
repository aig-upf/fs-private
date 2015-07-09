
#pragma once

#include <vector>
#include <ostream>

#include <problem_info.hxx>
#include <actions.hxx>

namespace fs0 {

class Problem;

//! Helper class - debugging purposes only
class PlanPrinter {
protected:
	const ActionPlan _plan;
public:
	PlanPrinter(const ActionPlan& plan) : _plan(plan) {}
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const PlanPrinter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	//! static helpers
	static void printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out);
	static void printPlan(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);
	static void printPlanJSON(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);	
};


} // namespaces
