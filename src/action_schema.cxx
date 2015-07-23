
#include <action_schema.hxx>
#include <problem.hxx>


namespace fs0 {

ActionSchema::ActionSchema(const std::string& name, const std::string& classname, const std::vector<AtomicFormulaSchema::cptr>& conditions, const std::vector<ActionEffectSchema::cptr>& effects)
	: _name(name), _classname(classname), _conditions(conditions), _effects(effects)
{}


ActionSchema::~ActionSchema() {
	for (const auto ptr:_conditions) delete ptr;
	for (const auto ptr:_effects) delete ptr;
}

std::ostream& ActionSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << "action " << _name << ":" << std::endl;
	os << "Preconditions:" << std::endl;
	for (auto elem:_conditions) {
		os << "\t" << *elem << std::endl;
	}
	os << "Effects:" << std::endl;
	for (auto elem:_effects) {
		os << "\t" << *elem << std::endl;
	}
	return os;
}

} // namespaces
