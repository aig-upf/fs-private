
$actionName(const ObjectIdxVector& binding,
            const ObjectIdxVector& derived,
            const std::vector<VariableIdxVector>& appRelevantVars,
			const std::vector<VariableIdxVector>& effRelevantVars,
			const std::vector<VariableIdxVector>& effAffectedVars) :
		CoreAction(binding, derived, appRelevantVars, effRelevantVars, effAffectedVars)
	{}