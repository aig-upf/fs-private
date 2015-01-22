
#include <relaxed_applicability_manager.hxx>
#include <action_manager.hxx>
#include <fact.hxx>

namespace fs0 {

	
RelaxedApplicabilityManager* RelaxedApplicabilityManager::createApplicabilityManager(const Action::vcptr& actions) {
	
	bool need_complex_applicability_manager = false;
	
	// We first analyse the type of actions preconditions
	for (const Action::cptr& action:actions) {
		for (const ScopedConstraint::cptr constraint:action->getConstraints()) {
			unsigned arity = constraint->getArity();
			if (arity == 0) {
				throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
			}
			
			if(arity == 1) {
				if (!dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint) || constraint->filteringType() != ScopedConstraint::Filtering::Unary) {
					throw std::runtime_error("Cannot handle this type of unary constraint in action preconditions.");
				}
			} else {
				need_complex_applicability_manager = true;
			}
		}
	}
	
	// And now instantiate the appropriate applicability manager
	if (!need_complex_applicability_manager) {
		// We now for sure that all action constraints are unary and can be safely casted to UnaryParametrizedScopedConstraint during the relaxed plan computation
		return new RelaxedApplicabilityManager();
	} else {
		return new CompleteApplicabilityManager();
	}
}

std::pair<bool, FactSetPtr> RelaxedApplicabilityManager::isApplicable(const Action& action, const State& seed, const DomainMap& domains) const {
	FactSetPtr causes = std::make_shared<FactSet>();
	
	for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
		if (!isProcedureApplicable(constraint, domains, seed, causes)) {
			return std::make_pair(false, nullptr);
		}
	}
	
	return std::make_pair(true, causes);
}

bool RelaxedApplicabilityManager::isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, FactSetPtr causes) const {
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

bool CompleteApplicabilityManager::isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, FactSetPtr causes) const {
	// Note that because we are using this type of applicability manager, we know for sure that some constraint will have arity > 1
	throw std::runtime_error("Hasn't been implemented yet");
	return true;
}

} // namespaces
