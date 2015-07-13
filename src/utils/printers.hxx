
#pragma once

#include <vector>
#include <ostream>

#include <problem_info.hxx>
#include <actions.hxx>

namespace fs0 {

class Problem;
class SupportedAction;

//! Helper class - debugging purposes only
class PlanPrinter {
protected:
	const ActionPlan& _plan;
public:
	PlanPrinter(const ActionPlan& plan) : _plan(plan) {}
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const PlanPrinter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	static void printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out);
	
	//! static helpers
	static void printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out);
	static void printPlan(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);
	static void printPlanJSON(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);	
};

class SupportedPlanPrinter {
protected:
	const std::set<SupportedAction>& _plan;
public:
	SupportedPlanPrinter(const std::set<SupportedAction>& plan) : _plan(plan) {}
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const SupportedPlanPrinter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		printSupportedPlan(_plan, os);
		return os;
	}
	
	static void printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out);
};

} // namespaces
