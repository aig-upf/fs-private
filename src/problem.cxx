
#include <problem.hxx>
#include <utils/logging.hxx>
#include <utils/printers/language.hxx>
#include <applicability/csp_formula_interpreter.hxx>
#include <applicability/direct_formula_interpreter.hxx>
#include <actions/applicable_action_set.hxx>
#include <actions/action_schema.hxx>
#include <actions/ground_action.hxx>

namespace fs0 {

std::unique_ptr<Problem> Problem::_instance = nullptr;
std::unique_ptr<ProblemInfo> Problem::_info = nullptr;


Problem::Problem(State* init, const std::vector<ActionSchema::cptr>& schemata, const fs::Formula* goal, const fs::Formula* state_constraints, TupleIndex&& tuple_index) :
	_tuple_index(std::move(tuple_index)),
	_init(init),
	_schemata(schemata),
	_ground(),
	_state_constraint_formula(state_constraints),
	_goal_formula(goal),
	_goal_sat_manager(FormulaInterpreter::create(_goal_formula, get_tuple_index()))
{}

Problem::~Problem() {
	for (const auto pointer:_schemata) delete pointer;
	for (const auto pointer:_ground) delete pointer;
	delete _state_constraint_formula;
	delete _goal_formula;
}

ApplicableActionSet Problem::getApplicableActions(const State& s) const {
	return ApplicableActionSet(ApplicabilityManager(getStateConstraints()), s, _ground);
}

std::ostream& Problem::print(std::ostream& os) const { 
	const fs0::ProblemInfo& info = getProblemInfo();
	os << "Planning Problem [domain: " << info.getDomainName() << ", instance: " << info.getInstanceName() <<  "]" << std::endl;
	
	os << "Goal Conditions:" << std::endl << "------------------" << std::endl;
	os << "\t" << print::formula(*getGoalConditions()) << std::endl;
	os << std::endl;
	
	os << "State Constraints:" << std::endl << "------------------" << std::endl;
	os << "\t" << print::formula(*getStateConstraints()) << std::endl;
	os << std::endl;
	
	os << "Action schemata" << std::endl << "------------------" << std::endl;
	for (const ActionSchema::cptr elem:_schemata) {
		os << *elem << std::endl;
	}
	os << std::endl;
	
	os << "Ground Actions: " << _ground.size();
	// os << std::endl << "------------------" << std::endl;
	// for (const GroundAction::cptr elem:_ground) {
	// 	os << *elem << std::endl;
	// }
	os << std::endl;
	
	return os;
}

} // namespaces
