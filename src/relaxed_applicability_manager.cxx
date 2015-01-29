
#include <relaxed_applicability_manager.hxx>
#include <action_manager.hxx>
#include <fact.hxx>

namespace fs0 {

bool RelaxedApplicabilityManager::checkActionNeedsCompleteApplicabilityManager(const Action::cptr action) {
	bool needs_generic_manager = false;
	for (const ScopedConstraint::cptr constraint:action->getConstraints()) {
		unsigned arity = constraint->getArity();
		if (arity == 0) {
			throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
		}
		else if(arity == 1) {
			if (!dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint) || constraint->filteringType() != ScopedConstraint::Filtering::Unary) {
				throw std::runtime_error("Cannot handle this type of unary constraint in action preconditions.");
			}
		} else {
			needs_generic_manager = true;
		}
	}
	return needs_generic_manager;
}

RelaxedApplicabilityManager* RelaxedApplicabilityManager::createApplicabilityManager(const Action::vcptr& actions) {
	for (const Action::cptr& action:actions) {
		if (checkActionNeedsCompleteApplicabilityManager(action)) {
			action->constructConstraintManager();
		}
	}
	return new RelaxedApplicabilityManager();
}

bool RelaxedApplicabilityManager::isApplicable(const Action& action, const State& seed, const DomainMap& domains, Fact::vctr& causes) const {
	ConstraintManager* manager = action.getConstraintManager();
	if (!manager) {
		// If the action has no associated manager, we know for sure that all of the action applicability constraints are unary.
		return unaryApplicable(action, seed, domains, causes);
	} else {
		return genericApplicable(manager, seed, domains, causes);
	}
}

bool RelaxedApplicabilityManager::unaryApplicable(const Action& action, const State& seed, const DomainMap& domains, Fact::vctr& causes) const {
	for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
		if (!isUnaryProcedureApplicable(constraint, domains, seed, causes)) return false;
	}
	return true;
}

bool RelaxedApplicabilityManager::isUnaryProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, Fact::vctr& causes) const {
	// Note that because we are using this type of applicability manager, we know for sure that the constraint must be a UnaryConstraint.
	
	// TODO - Optimize taking into account that the constraint is unary
	
	const VariableIdxVector& relevant = constraint->getScope();
	VariableIdx variable = relevant[0];
	bool cause_found = false;
	
	// Check first if the original value is applicable. This is slightly inefficient, but ensures that if the original values
	// make an action applicable, we won't waste time tracing the causes of other values that might make it applicable as well.
	const ObjectIdx value = seed.getValue(variable);
	if (constraint->isSatisfied({value})) {
		causes.push_back(Fact(variable, value));
		cause_found = true;
	}
	
	Domain& values = *(domains.at(variable));
	Domain new_domain;
	
	for (auto& value:values) {
		if (constraint->isSatisfied({value})) {
			if (!cause_found) { // We only want to insert one single (arbitrarily chosen) cause for the unary app constraint.
				causes.push_back(Fact(variable, value));
				cause_found = true;
			}
			new_domain.insert(new_domain.end(), value); // We'll insert the element at the end, since we're iterating in order.
		}
	}
	
	values = new_domain; // Update the values with those that satisfy the unary constraint.
	return cause_found;
}


bool RelaxedApplicabilityManager::genericApplicable(ConstraintManager* manager, const State& seed, const DomainMap& domains, Fact::vctr& causes) const {
	ScopedConstraint::Output o = manager->filter(domains);
	
	if(o == ScopedConstraint::Output::Failure || !ConstraintManager::checkConsistency(domains)) {
		return false;
	}
	
	// The CSP is _not_ inconsistent. To build the cause of applicability of the action, we pick arbitrary values among the remaining consistent values.
	// We favor the seed value, if it is among them.
	for (const auto& domain:domains) {
		VariableIdx variable = domain.first;
		
		ObjectIdx seed_value = seed.getValue(variable);
		if (domain.second->find(seed_value) == domain.second->end()) {  // If the original value makes the situation a goal, then we don't need to add anything for this variable.
			ObjectIdx value = *(domain.second->cbegin());
			causes.push_back(Fact(variable, value)); // Otherwise we simply select an arbitrary value.
		}
	}
	return true;
}

} // namespaces
