
#include <cassert>
#include <memory>
#include <utils/printers.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {

void Printers::printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out) {
	auto problemInfo = problem.getProblemInfo();
	for (int action:plan) {
		out << problemInfo->getActionName(action) << std::endl;
	}
}

void Printers::printPlan(const std::vector<int>& plan, const ProblemInfo::cptr& problemInfo, std::ostream& out) {
	for (int action:plan) {
		out << problemInfo->getActionName(action) << std::endl;
	}
}

} // namespaces
