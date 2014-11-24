

#include <action_manager.hxx>
#include <action_set_manager.hxx>
#include <actions.hxx>
#include <utils/cartesian_product.hxx>
#include <fact.hxx>


namespace aptk { namespace core {


bool RelaxedActionSetManager::isApplicable(JustifiedApplicableEntity& justified) const {
	
	for (unsigned idx = 0; idx < justified.getEntity().getNumApplicabilityProcedures(); ++idx) {
		if (!isProcedureApplicable(justified, idx)) {
			return false;
		}
	}
	return true;
}

bool RelaxedActionSetManager::isProcedureApplicable(JustifiedApplicableEntity& justified, unsigned procedureIdx) const {
	
	const ApplicableEntity& action = justified.getEntity(); 
	const VariableIdxVector& relevant = action.getApplicabilityRelevantVars(procedureIdx);
	
	assert(relevant.size() != 0); // Static applicability procedure that should have been detected in compilation time
	
	if(relevant.size() == 1) {  // micro-optimization
		return isMonadicProcedureApplicable(justified, procedureIdx);
	}
	
	auto values = extractPoint(justified, relevant);
	CartesianProductIterator it(values);
	
	for (; !it.ended(); ++it) {
		ProcedurePoint point = *it;
		if (action.isApplicable(procedureIdx, point)) {
			// We generate and insert all the facts that have made the action applicable
			for (unsigned i = 0; i < relevant.size(); ++i) {
				justified.addCause(Fact(relevant[i], point[i]));
			}
			// TODO - Here we should probably keep iterating to discover all the combinations that actually make the action applicable,
			// TODO - which are the only ones that should be taken into account for appliying the effect procedures.
			return true; // TODO We can exit here or we could keep checking to gather more possible causes of applicability.
		}
	}
	return false;
}

bool RelaxedActionSetManager::isMonadicProcedureApplicable(JustifiedApplicableEntity& justified, unsigned procedureIdx) const {
	
	const ApplicableEntity& action = justified.getEntity(); 
	const VariableIdxVector& relevant = action.getApplicabilityRelevantVars(procedureIdx);	
	assert(relevant.size() == 1);  // i.e. it is indeed monadic.
	const VariableIdx& relevantVar = relevant[0];
	
	bool oneCauseFound = false;
	
	// Check first if the original value is applicable. This is slightly inefficient, but ensures that if the original values
	// make an action applicable, we won't waste time tracing the causes of other values that might make it applicable as well.
	if (_originalState) {
		const auto& value = _originalState->getValue(relevantVar);
		auto originalPoint = { value };
		if (action.isApplicable(procedureIdx, originalPoint)) {
			justified.addCause(Fact(relevantVar, value));
			oneCauseFound = true;
		}
	}
	
	auto& values = extractMonadicPoint(justified, relevantVar);
	
	for (auto it = values.begin(); it != values.end();) {
		const auto& value = *it;
		if (action.isApplicable(procedureIdx, ProcedurePoint({value}))) {
			if (!oneCauseFound) {
				oneCauseFound = true;
				justified.addCause(Fact(relevantVar, value));// TODO TODO TODO We should actually insert more causes, but as elements of a disjunction, or probably build a formula with them.
			}
			++it;
		} else {
			it = values.erase(it);
		}
	}
	return oneCauseFound;
}

// TODO - THIS IS RATHER EXPENSIVE
std::vector<VariableIdxVector> RelaxedActionSetManager::extractPoint(const JustifiedApplicableEntity& justified, const VariableIdxVector& variables) const {
	// `values[i]` will contain all possible values in the given state for the variable with index `variables[i]`
	std::vector<VariableIdxVector> values;
	
	const auto& map = justified.getBinding().getMap();
	
	for (auto& idx:variables) {
		const auto& it = map.find(idx);
		if (it != map.end()) { // We have a cached list of values, and thus use it instead of the whole list of values.
			values.push_back(it->second);
		} else {
			const auto& set = _state.getValues(idx);  // TODO - CHANGE TO A PAIR OF ITERATORS OF THE UNDERLYING CONTAINER INSTEAD
			values.push_back(std::vector<ObjectIdx>(set.cbegin(), set.cend())); // TODO - THIS IS RATHER EXPENSIVE, but unfortunately sets are not indexed :-(
		}
	}
	return values;
}

std::vector<ObjectIdx>& RelaxedActionSetManager::extractMonadicPoint(JustifiedApplicableEntity& justified, VariableIdx variable) const {
	auto& map = justified.getBinding().getMap();
	auto it = map.find(variable);
	if (it != map.end()) {
		return it->second;
	}
	
	// Insert all the possible values in the cache
	const ObjectSet& state_values = _state.getValues(variable) ;
	auto res = map.insert(std::make_pair(variable, std::vector<ObjectIdx>(state_values.begin(), state_values.end())));
	return res.first->second; // return a reference to the actual vector of values.
}

void RelaxedActionSetManager::computeChangeset(const JustifiedAction& justified, Changeset& changeset) const {
// 	assert(isApplicable(justified)); // TODO - This should be a call to some method 'checkJustificationIsAdequate(justified)' that does not modify the object 'justified'
	
	for (unsigned idx = 0; idx < justified.getAction().getNumEffectProcedures(); ++idx) {
		computeProcedureChangeset(idx, justified, changeset);
	}
}

void RelaxedActionSetManager::computeProcedureChangeset(unsigned procedureIdx, const JustifiedAction& justified, Changeset& changeset) const {
	const VariableIdxVector& relevant = justified.getAction().getEffectRelevantVars(procedureIdx);
	
	if(relevant.size() == 0) {  // No need to pass any point.
		computeProcedurePointChangeset(procedureIdx, justified, relevant, {}, changeset);
	}	else if(relevant.size() == 1) {  // micro-optimization
		computeMonadicProcedureChangeset(procedureIdx, justified, changeset);
	} else {
		auto values = extractPoint(justified, relevant);
		CartesianProductIterator it(values);
		
		for (; !it.ended(); ++it) {
			computeProcedurePointChangeset(procedureIdx, justified, relevant, *it, changeset);
		}
	}
}

void RelaxedActionSetManager::computeProcedurePointChangeset(unsigned procedureIdx, const JustifiedAction& justified,
															 const VariableIdxVector& relevant, const ProcedurePoint& point, Changeset& changeset) const {
// 	SimpleActionSetManager::computeProcedurePointChangeset(procedureIdx, justified.getAction(), point, changeset);
	const CoreAction& action = justified.getAction();
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


void RelaxedActionSetManager::computeMonadicProcedureChangeset(unsigned procedureIdx, const JustifiedAction& justified, Changeset& changeset) const {
	const VariableIdxVector& relevant = justified.getAction().getEffectRelevantVars(procedureIdx);
	assert(relevant.size() == 1);
	VariableIdx rel = relevant[0];
	
	const auto& map = justified.getBinding().getMap();
	const auto it = map.find(rel);
	if (it != map.end()) { // We have a cached list of values, and thus use it instead of the whole list of values.
		for (ObjectIdx val:it->second) {
			computeProcedurePointChangeset(procedureIdx, justified, relevant, ProcedurePoint({val}), changeset);
		}
	} else {
		for (ObjectIdx val:_state.getValues(rel)) {
			computeProcedurePointChangeset(procedureIdx, justified, relevant, ProcedurePoint({val}), changeset);
		}
	}
}


} } // namespaces
