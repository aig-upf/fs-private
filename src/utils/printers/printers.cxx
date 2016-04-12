#include <memory>
#include <utils/printers/printers.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/relaxed_plan_extractor.hxx>

namespace fs0 {


void PlanPrinter::print(const std::vector<GroundAction::IdType>& plan, std::ostream& out) {
	const auto& actions = Problem::getInstance().getGroundActions();
	for (auto action_id:plan) {
		const GroundAction& action = *actions.at(action_id);
		out << print::action_header(action) << " " << std::endl;
	}
}

void PlanPrinter::print(const std::vector<LiftedActionID>& plan, std::ostream& out) {
	// This ideally should be conflated with print::plan, but currently we have different plan types :-(
	for (const auto& element:plan) {
		out << element << " " << std::endl;
	}
}

void PlanPrinter::print_json(const std::vector<LiftedActionID>& plan, std::ostream& out) {
	std::vector<std::string> names;
	for (const auto& elem:plan) {
		std::ostringstream stream;
		stream << elem;
		names.push_back(stream.str());
	}
	print_json(names, out);
}

void PlanPrinter::print_json(const std::vector<GroundAction::IdType>& plan, std::ostream& out) {
	std::vector<std::string> names;
	const auto& actions = Problem::getInstance().getGroundActions();
	for (const auto& action_id:plan) {
		std::ostringstream stream;
		stream << print::action_header(*actions.at(action_id));
		names.push_back(stream.str());
	}
	print_json(names, out);
}

void PlanPrinter::print_json(const std::vector<std::string>& action_names, std::ostream& out) {
	out << "[";
	for ( unsigned k = 0; k < action_names.size(); k++ ) {
		out << "\"" <<  action_names[k] << "\"";
		if ( k < action_names.size() - 1 ) out << ", ";
	}
	out << "]";
}


std::ostream& PlanPrinter::print(std::ostream& os) const {
	print(_plan, os);
	return os;
}


namespace print {

std::ostream& plan::print(std::ostream& os) const {
	for (const ActionID* action:_plan) {
		os << *action << " ";
	}
	return os;
}

void supported_plan::printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out) {
	for (const auto& element:plan) {
		out << element << " " << std::endl;
	}
}

}

} // namespaces
