
#include <cassert>
#include <memory>
#include <utils/printers/printers.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <heuristics/relaxed_plan/relaxed_plan_extractor.hxx>

namespace fs0 {


void PlanPrinter::printPlan(const std::vector<GroundAction::IdType>& plan, const Problem& problem, std::ostream& out) {
	for (auto action:plan) {
		out << print::action_name(action) << std::endl;
	}
}

void PlanPrinter::printPlanJSON(const std::vector<GroundAction::IdType>& plan, const Problem& problem, std::ostream& out) {
	out << "[";
	for ( unsigned k = 0; k < plan.size(); k++ ) {
		out << "\"" <<  print::action_name(plan[k]) << "\"";
		if ( k < plan.size() - 1 ) out << ", ";
	}
	out << "]";
}

std::ostream& PlanPrinter::print(std::ostream& os) const {
	printPlan(_plan, Problem::getInstance(), os);
	return os;
}

void SupportedPlanPrinter::printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out) {
	for (const auto& element:plan) {
		out << element << std::endl;
	}
}


} // namespaces
