
#include <action_manager.hxx>
#include <relaxed_action_set_manager.hxx>
#include <actions.hxx>
#include <utils/cartesian_product.hxx>
#include <fact.hxx>


namespace aptk { namespace core {
	
DomainSet RelaxedActionSetManager::projectValues(const RelaxedState& state, const ApplicableEntity& action) const {
	DomainSet projection;
	for (VariableIdx var:action.getAllRelevantVariables()) {
		const auto& all = state.getValues(var);
		projection.insert(std::make_pair(var, DomainSetVector(all.cbegin(), all.cend()))); // This is expensive, but unfortunately sets are not indexed :-(
	}
	return projection;
}

// bool RelaxedActionSetManager::isApplicable(JustifiedApplicableEntity& justified) const {
std::pair<bool, FactSetPtr> RelaxedActionSetManager::isApplicable(const ApplicableEntity& entity, DomainSet& domains) const {
	FactSetPtr causes = std::make_shared<FactSet>();
	for (unsigned idx = 0; idx < entity.getNumApplicabilityProcedures(); ++idx) {
		if (!isProcedureApplicable(entity, domains, idx, causes)) {
			return std::make_pair(false, causes);
		}
	}
	return std::make_pair(true, causes);
}

bool RelaxedActionSetManager::isProcedureApplicable(const ApplicableEntity& entity, DomainSet& domains, unsigned procedureIdx, FactSetPtr causes) const {
	
	const VariableIdxVector& relevant = entity.getApplicabilityRelevantVars(procedureIdx);
	
	if(relevant.size() > 1) {
		// For the generic, n-ary case, we should move to the more sophisticated CSP manager.
		throw std::runtime_error("Action applicability procedures of arity > 1 are currently unsupported");
	} else if (relevant.size() == 0) {
		throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
	}
	
	const VariableIdx& variable = relevant[0];
	bool cause_found = false;
	
	// Check first if the original value is applicable. This is slightly inefficient, but ensures that if the original values
	// make an action applicable, we won't waste time tracing the causes of other values that might make it applicable as well.
	if (seed) {
		const ObjectIdx value = seed->getValue(variable);
		auto originalPoint = { value };
		if (entity.isApplicable(procedureIdx, originalPoint)) {
			causes->insert(Fact(variable, value));
			cause_found = true;
		}
	}
	
	auto& values = domains.at(variable);
	DomainSetVector newProjection;
	
	for (auto& value:values) {
		if (entity.isApplicable(procedureIdx, ProcedurePoint({value}))) {
			if (!cause_found) { // We only want to insert one single (arbitrarily chosen) cause for the unary app constraint.
				causes->insert(Fact(variable, value));
				cause_found = true;
			}
			newProjection.insert(newProjection.end(), value); // We'll insert the element at the end, since we're iterating in order.
		}
	}
	
	values = newProjection; // Update the values with those that satisfy the unary constraint.
	return cause_found;
}

LightDomainSet RelaxedActionSetManager::extractPoint(DomainSet& domains, const VariableIdxVector& variables) const {
	// `values[i]` will contain all possible values in the given state for the variable with index `variables[i]`
	LightDomainSet values;
	for (auto index:variables) {
		values.push_back(&(domains[index]));
	}
	return values;
}

void RelaxedActionSetManager::computeChangeset(const CoreAction& action, const DomainSet& domains, Changeset& changeset) const {
	for (unsigned idx = 0; idx < action.getNumEffectProcedures(); ++idx) {
		computeProcedureChangeset(idx, action, domains, changeset);
	}
}

void RelaxedActionSetManager::computeProcedureChangeset(unsigned procedureIdx, const CoreAction& action, const DomainSet& domains, Changeset& changeset) const {
	const VariableIdxVector& relevant = action.getEffectRelevantVars(procedureIdx);
	
	if(relevant.size() == 0) {  // No need to pass any point.
		computeProcedurePointChangeset(procedureIdx, action, relevant, {}, changeset);
	}	else if(relevant.size() == 1) {  // micro-optimization
		for (ObjectIdx val:domains.at(relevant[0])) { // Add to the changeset for every allowed value of the relevant variable
			computeProcedurePointChangeset(procedureIdx, action, relevant, ProcedurePoint({val}), changeset);
		}
	} else { // The general, n-ary case. We need to iterate over the cartesian product of the allowed values for the relevant variables.
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

void RelaxedActionSetManager::computeProcedurePointChangeset(unsigned procedureIdx, const CoreAction& action,
															 const VariableIdxVector& relevant, const ProcedurePoint& point, Changeset& changeset) const {
	const VariableIdxVector& affectedVars = action.getEffectAffectedVars(procedureIdx);
	ProcedurePoint affectedValues(affectedVars.size());
	// TODO - WHAT HAPPENS IF THE POINT WAS ACTUALLY NOT AFFECTED BY THE PROCEDURE??
	// TODO - WE'LL BE UNDERSTANDING THAT THE VALUE BECAME 0, WHILE ACTUALLY NOONE TOUCHED IT.
	// TODO - SOMETHING LIKE A MAP MIGHT HELP SOLVING IT, BUT IT'LL BE MORE EXPENSIVE.
	// TODO - OR A VECTOR OF PAIRS <AFFECTED_IDX, NEW_VALUE>
	action.applyEffectProcedure(procedureIdx, point, affectedValues);
	
	// Add as extra causes all the relevant facts of the effect procedure.
	FactSetPtr extraCauses = std::make_shared<FactSet>();
	for (unsigned i = 0; i < relevant.size(); ++i) {
		extraCauses->insert(Fact(relevant[i], point[i]));
	}
	
	// Zip the new values for the affected variables into new facts and add them into the changeset.
	for (unsigned i = 0; i < affectedVars.size(); ++i) {
		changeset.add(Fact(affectedVars[i], affectedValues[i]), extraCauses);
	}	
}




} } // namespaces
