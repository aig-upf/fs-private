
$actionName(const VariableIdxVector& binding,
            const VariableIdxVector& derived,
            const std::vector<VariableIdxVector>& appRelevantVars,
			const std::vector<VariableIdxVector>& effRelevantVars,
			const std::vector<VariableIdxVector>& effAffectedVars) :
		CoreAction(binding, derived, appRelevantVars, effRelevantVars, effAffectedVars)
	{}