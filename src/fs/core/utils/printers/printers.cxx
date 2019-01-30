
#include <memory>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/printers/printers.hxx>

namespace fs0 {


void PlanPrinter::print(const std::vector<GroundAction::IdType>& plan, std::ostream& out) {
	const auto& actions = Problem::getInstance().getGroundActions();
	for (auto action_id:plan) {
		const GroundAction& action = *actions.at(action_id);
		out << print::strips_action_header(action) << std::endl;
	}
}


void PlanPrinter::print_json(const std::vector<GroundAction::IdType>& plan, std::ostream& out) {
	std::vector<std::string> names;
	const auto& actions = Problem::getInstance().getGroundActions();
	for (const auto& action_id:plan) {
		names.push_back(printer() << print::action_header(*actions.at(action_id)));
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


} // namespaces

