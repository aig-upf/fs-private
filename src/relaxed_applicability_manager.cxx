
#include <relaxed_applicability_manager.hxx>
#include <action_manager.hxx>
#include <fact.hxx>

namespace fs0 {

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
	
	const VariableIdxVector& relevant = constraint->getScope();
	
	if(relevant.size() > 1) {
		// For the generic, n-ary case, we should move to a more sophisticated CSP manager.
		throw std::runtime_error("Action applicability procedures of arity > 1 are currently unsupported");
	} else if (relevant.size() == 0) {
		throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
	}
	
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



} // namespaces
