
#include <problem.hxx>

namespace fs0 {
	
const Problem* Problem::_instance = 0;

Problem::Problem() :
	stateConstraints(),
	goalConstraints(),
	appManager(stateConstraints),
	effManager(stateConstraints)
{}

Problem::~Problem() {
	for (const auto& ctr:stateConstraints) delete ctr;
	for (const auto& ctr:goalConstraints) delete ctr;
} 
	

void Problem::bootstrap() {
	// Create the constraint manager
	ctrManager = std::make_shared<PlanningConstraintManager>(goalConstraints, stateConstraints);
}
	
	
} // namespaces
