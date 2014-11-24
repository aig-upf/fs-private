

class ComponentFactory {
public:
    static CoreAction::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const std::vector<VariableIdxVector>& effAffectedVars
    ) {
        CoreAction* aptr;
        if (false) {}
        ${lines}
        else throw std::runtime_error("Unknown action name.");

        return CoreAction::cptr(aptr);
    }

    static ApplicableEntity::cptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) {
        ApplicableEntity* gptr = new GoalEvaluator(appRelevantVars);
        return ApplicableEntity::cptr(gptr);
    }

};