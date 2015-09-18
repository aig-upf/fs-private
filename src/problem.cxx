
#include <problem.hxx>
#include <sstream>
#include <problem_info.hxx>
#include <utils/logging.hxx>
#include <utils/printers/language.hxx>

namespace fs0 {

std::unique_ptr<Problem> Problem::_instance = nullptr;

Problem::Problem() :
	_problemInfo(nullptr),
	_stateConstraints(),
	_goalConditions()
{
}

Problem::~Problem() {
	delete _problemInfo;
	for (const auto pointer:_schemata) delete pointer;
	for (const auto pointer:_ground) delete pointer;
	for (const auto pointer:_stateConstraints) delete pointer;
	for (const auto pointer:_goalConditions) delete pointer;
}

ApplicableActionSet Problem::getApplicableActions(const State& s) const {
	return ApplicableActionSet(ApplicabilityManager(getStateConstraints()), s, _ground);
}

std::string Problem::get_action_name(unsigned action) const {
	return _ground[action]->getFullName();
}


std::ostream& Problem::print(std::ostream& os) const { 
	const fs0::ProblemInfo& info = getProblemInfo();
	os << "Planning Problem [domain: " << info.getDomainName() << ", instance: " << info.getInstanceName() <<  "]" << std::endl;
	
	os << "Goal Conditions:" << std::endl << "------------------" << std::endl;
	for (const auto formula:getGoalConditions()) os << "\t" << print::formula(*formula) << std::endl;
	os << std::endl;
	
	os << "State Constraints:" << std::endl << "------------------" << std::endl;
	for (const auto formula:getStateConstraints()) os << "\t" << print::formula(*formula) << std::endl;
	os << std::endl;
	
	os << "Action schemata" << std::endl << "------------------" << std::endl;
	for (const ActionSchema::cptr elem:_schemata) {
		os << *elem << std::endl;
	}
	os << std::endl;
	
	os << "Ground Actions" << std::endl << "------------------" << std::endl;
	for (const GroundAction::cptr elem:_ground) {
		os << *elem << std::endl;
	}
	os << std::endl;
	
	return os;
}

} // namespaces
