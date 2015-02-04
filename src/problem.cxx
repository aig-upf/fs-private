
#include <problem.hxx>
#include <constraints/scoped_constraint.hxx>
#include <actions.hxx>

namespace fs0 {
	
const Problem* Problem::_instance = 0;

Problem::Problem() :
	stateConstraints(),
	goalConstraints(),
	effManager()
{}

Problem::~Problem() {
	for (const auto pointer:_actions) delete pointer;
	for (const auto pointer:stateConstraints) delete pointer;
	for (const auto pointer:goalConstraints) delete pointer;
	delete appManager;
} 
	

void Problem::bootstrap() {
	// Compile the constraints if necessary
	compileActionConstraints();
	
	// Create the constraint manager
	ctrManager = std::make_shared<PlanningConstraintManager>(goalConstraints, stateConstraints);
	
	// Creates the appropriate applicability manager depending on the type and arity of action precondition constraints.
	appManager = RelaxedApplicabilityManager::createApplicabilityManager(_actions);
}

void Problem::compileActionConstraints() {
	
	for (Action::ptr action:_actions) {
		
		std::vector<ScopedConstraint::cptr>& constraints = action->getConstraints();
		for (unsigned i = 0; i < constraints.size(); ++i) {
			if(UnaryParametrizedScopedConstraint* p = dynamic_cast<UnaryParametrizedScopedConstraint*>(constraints[i])) {
				constraints[i] = new CompiledUnaryConstraint(*p, *_problemInfo);
 				delete p;
			} else if (BinaryParametrizedScopedConstraint* p = dynamic_cast<BinaryParametrizedScopedConstraint*>(constraints[i])) {
				constraints[i] = new CompiledBinaryConstraint(*p, *_problemInfo);
 				delete p;
			}
		}
		
	}
}


	
	
} // namespaces
