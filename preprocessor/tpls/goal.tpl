
class GoalEvaluator : public ApplicableEntity {

public:
    GoalEvaluator(const std::vector<VariableIdxVector>& appRelevantVars) :
            ApplicableEntity(appRelevantVars)
        {}

    virtual bool isApplicable(unsigned procedureIdx, const ProcedurePoint& relevant) const {
		$applicability_code_switch
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}
};
