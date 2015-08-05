
#include <problem.hxx>
#include <sstream>
#include <problem_info.hxx>
#include <utils/logging.hxx>
#include <constraints/registry.hxx>
#include <utils/printers/registry.hxx>

namespace fs0 {

const Problem* Problem::_instance = 0;

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


void Problem::bootstrap() {
	// Safety check
	if (_goalConditions.empty()) {
		throw std::runtime_error("No goal specification detected. The problem is probably being bootstrapped before having been fully initialized with the per-instance generate() procedure"); 
	}
	
	FDEBUG("components", "Bootstrapping problem with following external component repository\n" << print::logical_registry(LogicalComponentRegistry::instance()));
}


ApplicableActionSet Problem::getApplicableActions(const State& s) const {
	return ApplicableActionSet(ApplicabilityManager(getStateConstraints()), s, _ground);
}


std::ostream& Problem::print(std::ostream& os) const { 
	const fs0::ProblemInfo& info = getProblemInfo();
	os << "Planning Problem [domain:" << info.getDomainName() << ", instance: " << info.getInstanceName() <<  "]" << std::endl;
	
	os << "Action schemata" << std::endl;
	for (const ActionSchema::cptr elem:_schemata) {
		os << *elem << std::endl;
	}
	
	os << "Ground Actions" << std::endl;
	for (const GroundAction::cptr elem:_ground) {
		os << *elem << std::endl;
	}
	
	return os;
}

} // namespaces
