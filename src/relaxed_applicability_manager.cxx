
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

std::pair<bool, FactSetPtr> RelaxedApplicabilityManager::isApplicable(const Action& action, const State& seed, const DomainMap& domains) const {
	ConstraintManager* manager = action.getConstraintManager();
	if (!manager) {
		// If the action has no associated manager, we know for sure that all of the action applicability constraints are unary.
		return unaryApplicable(action, seed, domains);
	} else {
		return genericApplicable(manager, seed, domains);
	}
}

std::pair<bool, FactSetPtr> RelaxedApplicabilityManager::unaryApplicable(const Action& action, const State& seed, const DomainMap& domains) const {
	FactSetPtr causes = std::make_shared<FactSet>();
	
	for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
		if (!isUnaryProcedureApplicable(constraint, domains, seed, causes)) {
			return std::make_pair(false, nullptr);
		}
	}
	
	return std::make_pair(true, causes);
}

bool RelaxedApplicabilityManager::isUnaryProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, FactSetPtr causes) const {
	// Note that because we are using this type of applicability manager, we know for sure that the constraint must be a UnaryConstraint.
	
	// TODO - Optimize taking into account that the constraint is unary
	
	const VariableIdxVector& relevant = constraint->getScope();
	VariableIdx variable = relevant[0];
	bool cause_found = false;
	
	// Check first if the original value is applicable. This is slightly inefficient, but ensures that if the original values
	// make an action applicable, we won't waste time tracing the causes of other values that might make it applicable as well.
	const ObjectIdx value = seed.getValue(variable);
	if (constraint->isSatisfied({value})) {
		causes->insert(Fact(variable, value));
		cause_found = true;
	}
	
	Domain& values = *(domains.at(variable));
	Domain new_domain;
	
	for (auto& value:values) {
		if (constraint->isSatisfied({value})) {
			if (!cause_found) { // We only want to insert one single (arbitrarily chosen) cause for the unary app constraint.
				causes->insert(Fact(variable, value));
				cause_found = true;
			}
			new_domain.insert(new_domain.end(), value); // We'll insert the element at the end, since we're iterating in order.
		}
	}
	
	values = new_domain; // Update the values with those that satisfy the unary constraint.
	return cause_found;
}


std::pair<bool, FactSetPtr> RelaxedApplicabilityManager::genericApplicable(ConstraintManager* manager, const State& seed, const DomainMap& domains) const {
	FactSetPtr causes = std::make_shared<FactSet>();
	ScopedConstraint::Output o = manager->filter(domains);
	
	if(o == ScopedConstraint::Output::Failure || !ConstraintManager::checkConsistency(domains)) {
		return std::make_pair(false, nullptr);
	}
	
	// The CSP is _not_ inconsistent. To build the cause of applicability of the action,
	// we pick arbitrary values among the remaining consistent values.
	// We favor the seed value, if it is among them.
	for (const auto& domain:domains) {
		VariableIdx variable = domain.first;
		
		ObjectIdx seed_value = seed.getValue(variable);
		if (domain.second->find(seed_value) == domain.second->end()) {  // If the original value makes the situation a goal, then we don't need to add anything for this variable.
			ObjectIdx value = *(domain.second->cbegin());
			causes->insert(Fact(variable, value)); // Otherwise we simply select an arbitrary value.
		}
	}
	return std::make_pair(true, causes);
}

} // namespaces
