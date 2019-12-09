
#include <memory>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/heuristics/relaxed_plan/relaxed_plan_extractor.hxx>

namespace fs0 {


void PlanPrinter::print(const std::vector<const GroundAction*>& plan, std::ostream& out) {
    for (auto action:plan) {
        out << print::strips_action_header(*action) << std::endl;
    }
}

void PlanPrinter::print_json(const std::vector<const GroundAction*>& plan, std::ostream& out) {
    std::vector<std::string> names;
    for (const auto& action:plan) {
        names.push_back(printer() << print::action_header(*action));
    }
    print_json(names, out);
}

void PlanPrinter::print_json(const std::vector<std::string>& action_names, std::ostream& out) {
	out << "[";
	for (std::size_t k = 0; k < action_names.size(); k++) {
		out << "\"" <<  action_names[k] << "\"";
		if ( k < action_names.size() - 1 ) out << ", ";
	}
	out << "]";
}


} // namespaces



namespace fs0 { namespace print {

std::ostream&
plan::print(std::ostream& os) const {
	for (const ActionID* action:_plan) {
		os << *action << " ";
	}
	return os;
}

void
supported_plan::printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out) {
	for (const auto& element:plan) {
		out << element << " " << std::endl;
	}
}

std::ostream&
support::print(std::ostream& os) const {
	const AtomIndex& index = Problem::getInstance().get_tuple_index();
	for (AtomIdx tuple:_support) {
		const Atom& atom = index.to_atom(tuple);
		os << atom << ", ";
	}
	return os;
}


std::ostream&
changeset::print(std::ostream& os) const {
	os << "<";
	for (unsigned i = 0, s = static_cast<unsigned>(_changeset.size()); i < s; ) {
		os << _changeset[i];
		if (++i < s) os << ", ";
	}
	os << ">";
	return os;
}

}} // namespaces
