
#include <cassert>
#include <memory>
#include <utils/printers.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <heuristics/relaxed_plan_extractor.hxx>

namespace fs0 {

void PlanPrinter::printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out) {
	const ProblemInfo& problemInfo = problem.getProblemInfo();
	for (int action:plan) {
		out << problemInfo.getActionName(action) << std::endl;
	}
}

void PlanPrinter::printPlan(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out) {
	for (int action:plan) {
		out << problemInfo.getActionName(action) << std::endl;
	}
}

void PlanPrinter::printPlanJSON(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out) {
	out << "[";
	for ( unsigned k = 0; k < plan.size(); k++ ) {
		out << "\"" << problemInfo.getActionName(plan[k]) << "\"";
		if ( k < plan.size() - 1 ) out << ", ";
	}
	out << "]";
}

std::ostream& PlanPrinter::print(std::ostream& os) const {
	printPlan(_plan, *Problem::getCurrentProblem(), os);
	return os;
}

void SupportedPlanPrinter::printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out) {
	for (const auto& element:plan) {
		out << element << std::endl;
	}
}

} // namespaces
