
#include <cassert>
#include <memory>
#include <utils/printers.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {

void Printers::printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out) {
	const ProblemInfo& problemInfo = problem.getProblemInfo();
	for (int action:plan) {
		out << problemInfo.getActionName(action) << std::endl;
	}
}

void Printers::printPlan(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out) {
	for (int action:plan) {
		out << problemInfo.getActionName(action) << std::endl;
	}
}

void Printers::printPlanJSON(const std::vector<Action::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out) {
	out << "[";
	for ( unsigned k = 0; k < plan.size(); k++ ) {
		out << "\"" << problemInfo.getActionName(plan[k]) << "\"";
		if ( k < plan.size() - 1 ) out << ", ";
	}
	out << "]";
}

} // namespaces
