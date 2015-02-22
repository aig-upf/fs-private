
#include <problem.hxx>
#include <constraints/scoped_constraint.hxx>
#include <actions.hxx>

namespace fs0 {
	
const Problem* Problem::_instance = 0;

Problem::Problem(const std::string& data_dir) :
	_problemInfo(data_dir),
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
	// Add the necessary (unary) constraints to keep the appropriate values, if any, within their bounds.
	addDomainBoundConstraints();
	
	// Compile the constraints if necessary
	compileConstraints();
	
	// Create the constraint manager
	ctrManager = std::make_shared<PlanningConstraintManager>(goalConstraints, stateConstraints);
	
	// Creates the appropriate applicability manager depending on the type and arity of action precondition constraints.
	appManager = RelaxedApplicabilityManager::createApplicabilityManager(_actions);
}

void Problem::addDomainBoundConstraints() {
	unsigned num_bconstraints = 0;
	
	std::cout << "Generating unary constraints for bounded domains..." << std::endl;
	
	for (Action::ptr action:_actions) {
		
		for (const ScopedEffect::cptr effect:action->getEffects()) {
			VariableIdx affected = effect->getAffected();
			const VariableIdxVector& relevant = effect->getScope();
			
			if (relevant.size() > 1) throw std::runtime_error("Action effect procedures of arity > 1 are currently unsupported");
			
			if (_problemInfo.hasVariableBoundedDomain(affected)) {
				if (relevant.size() == 0) {
					ObjectIdx value = effect->apply();
					if (!_problemInfo.checkValueIsValid(affected, value)) throw std::runtime_error("A 0-ary effect produces out-of-bounds variable values");
				} else { // unary effect
					const UnaryScopedEffect* eff = dynamic_cast<const UnaryScopedEffect *>(effect);
					assert(eff);
					ScopedConstraint::cptr constraint = new DomainBoundsConstraint(eff, _problemInfo);
					action->addConstraint(constraint);
					++num_bconstraints;
				}
			}
		}
	}
	
	std::cout << "Added a total of " << num_bconstraints << " unary constraints for bounded domains to the " <<  _actions.size() << " problem actions." << std::endl;
}

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

SimpleApplicableActionSet Problem::getApplicableActions(const State& s) const {
	return SimpleApplicableActionSet(StandardApplicabilityManager(s, getConstraints()), _actions);
}

} // namespaces
