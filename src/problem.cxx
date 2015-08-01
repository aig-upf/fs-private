
#include <problem.hxx>
#include <sstream>
#include <problem_info.hxx>
#include <utils/logging.hxx>

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
	
// 	gecode::registerTranslators();
	
	// Compile the constraints if necessary
// 	compileConstraints();
}


/*
void Problem::compileConstraints() {
	unsigned num_compiled = 0;
	for (Action::ptr action:_actions) {
		num_compiled += compileConstraintVector(action->getConstraints());
	}
	num_compiled += compileConstraintVector(stateConstraints);
	num_compiled += compileConstraintVector(goalConstraints);

	std::cout << "Compiled a total of " << num_compiled << " constraints." << std::endl;
}

unsigned Problem::compileConstraintVector(ScopedConstraint::vcptr& constraints) const {
	unsigned num_compiled = 0;
	for (unsigned i = 0; i < constraints.size(); ++i) {
		ScopedConstraint::cptr compiled = constraints[i]->compile(_problemInfo);
		if (compiled) { // The constraint type requires pre-compilation
			delete constraints[i];
			constraints[i] = compiled;
			++num_compiled;
		}
	}
	return num_compiled;
}
*/

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
