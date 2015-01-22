
#include <relaxed_action_set_manager.hxx>
#include <action_manager.hxx>
#include <fact.hxx>

namespace fs0 {

std::pair<bool, FactSetPtr> RelaxedActionSetManager::isApplicable(const Action& action, const DomainMap& domains) const {
	FactSetPtr causes = std::make_shared<FactSet>();
	
	for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
		if (!isProcedureApplicable(constraint, domains, causes)) {
			return std::make_pair(false, nullptr);
		}
	}
	
	return std::make_pair(true, causes);
}

bool RelaxedActionSetManager::isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, FactSetPtr causes) const {
	
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
	if (seed) {
		const ObjectIdx value = seed->getValue(variable);
		if (constraint->isSatisfied({value})) {
			causes->insert(Fact(variable, value));
			cause_found = true;
		}
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


void RelaxedActionSetManager::computeChangeset(const Action& action, const DomainMap& domains, Changeset& changeset) const {
	
	for (const ScopedEffect::cptr effect:action.getEffects()) {
		
		const VariableIdxVector& relevant = effect->getScope();
		
		
		if(relevant.size() == 0) {  // No need to pass any point.
			changeset.add(Fact(effect->getAffected(), effect->apply({})), std::make_shared<FactSet>());
		}
		else if(relevant.size() == 1) {  // micro-optimization
			for (ObjectIdx val:*(domains.at(relevant[0]))) { // Add to the changeset for every allowed value of the relevant variable
				computeProcedurePointChangeset(effect, relevant, {val}, changeset);
			}
		}
		
		else { // The general, n-ary case. We need to iterate over the cartesian product of the allowed values for the relevant variables.
				throw std::runtime_error("Action effect procedures of arity > 1 are currently unsupported");
				// TODO - They are implementable - just need to finish the cartesianproductiterator
				// For that, we need to have the values in domains in 
		// 		LightDomainSet values = extractPoint(domains, relevant);
		// 		CartesianProductIterator it(values);
		// 		
		// 		for (; !it.ended(); ++it) {
		// 			computeProcedurePointChangeset(procedureIdx, action, relevant, *it, changeset);
		// 		}
		}
	}
}


void RelaxedActionSetManager::computeProcedurePointChangeset(
	const ScopedEffect::cptr effect, const VariableIdxVector& relevant, const ObjectIdxVector& values, Changeset& changeset) const {
	
	// TODO - Note that this won't work for conditional effects where an action might have no effect at all
	VariableIdx affected = effect->getAffected();

	// Add as extra causes all the relevant facts of the effect procedure.
	FactSetPtr extraCauses = std::make_shared<FactSet>();
	for (unsigned i = 0; i < relevant.size(); ++i) {
		extraCauses->insert(Fact(relevant[i], values[i]));
	}
	changeset.add(Fact(affected, effect->apply(values)), extraCauses);
}


} // namespaces
