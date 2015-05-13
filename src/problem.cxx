
#include <problem.hxx>
#include <constraints/scoped_constraint.hxx>
#include <actions.hxx>
#include <relaxed_action_manager.hxx>

namespace fs0 {
	
const Problem* Problem::_instance = 0;

Problem::Problem(const std::string& data_dir) :
	_problemInfo(data_dir),
	stateConstraints(),
	goalConstraints()
{}

Problem::~Problem() {
	for (const auto pointer:_actions) delete pointer;
	for (const auto pointer:stateConstraints) delete pointer;
	for (const auto pointer:goalConstraints) delete pointer;
} 
	

void Problem::bootstrap() {
	// Add the necessary (unary) constraints to keep the appropriate values, if any, within their bounds.
	addDomainBoundConstraints();
	
	// Compile the constraints if necessary
	compileConstraints();
	
	// Create the constraint manager
	ctrManager = std::make_shared<PlanningConstraintManager>(goalConstraints, stateConstraints);
	
	// Creates the appropriate applicability manager depending on the type and arity of action precondition constraints.
	ActionManagerFactory::instantiateActionManager(_actions);
}

//! For every action, if any of its effects affects a bounded-domain variable, we place
//! an "automatic" "bounds constraint" upon the variables which are relevant to the effect,
//! so that they won't generate an out-of-bounds value.
void Problem::addDomainBoundConstraints() {
	unsigned num_bconstraints = 0;
	
	std::cout << "Generating unary constraints for bounded domains..." << std::endl;
	
	for (Action::ptr action:_actions) {
		
		for (const ScopedEffect::cptr effect:action->getEffects()) {
			VariableIdx affected = effect->getAffected();
			if (!_problemInfo.hasVariableBoundedDomain(affected)) continue;

			// We process the creation of the bound-constraint differently  for each arity
			unsigned arity = effect->getScope().size();
			if (arity == 0) {
				if (!_problemInfo.checkValueIsValid(effect->apply()))
					throw std::runtime_error("A 0-ary effect produces out-of-bounds variable values");
			} else if (arity == 1) {
				const auto* casted_effect = dynamic_cast<const UnaryScopedEffect *>(effect);
				assert(casted_effect);
				action->addConstraint(new UnaryDomainBoundsConstraint(casted_effect, _problemInfo));
			} else if (arity == 2) {
				const auto* casted_effect = dynamic_cast<const BinaryScopedEffect *>(effect);
				assert(casted_effect);
				action->addConstraint(new BinaryDomainBoundsConstraint(casted_effect, _problemInfo));
			} else {
				throw UnimplementedFeatureException("Action effect procedures of arity > 2 are currently unsupported");
			}

			++num_bconstraints;
		}
	}
	
	std::cout << "Added a total of " << num_bconstraints << " bound-constraints to the " <<  _actions.size() << " problem actions." << std::endl;
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
