
#include <problem.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {
	
const Problem* Problem::_instance = 0;

Problem::Problem() :
	stateConstraints(),
	goalConstraints(),
	effManager()
{}

Problem::~Problem() {
	for (const auto& pointer:stateConstraints) delete pointer;
	for (const auto& pointer:goalConstraints) delete pointer;
	delete appManager;
} 
	

void Problem::bootstrap() {
	// Create the constraint manager
	ctrManager = std::make_shared<PlanningConstraintManager>(goalConstraints, stateConstraints);
	
	// Creates the appropriate applicability manager depending on the type and arity of action precondition constraints.
	appManager = RelaxedApplicabilityManager::createApplicabilityManager(_actions);
}




	
	
} // namespaces
