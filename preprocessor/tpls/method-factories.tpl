

class ComponentFactory {
public:
    static Action::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const std::vector<VariableIdxVector>& effAffectedVars
    ) {
        Action* action = nullptr;
        if (false) {}
        ${lines}
        else throw std::runtime_error("Unknown action name.");

        return action;
    }

    static ScopedConstraint::vcptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) {
    const ObjectIdxVector binding;  // The goal has empty binding
		return {
		    ${goal_constraint_instantiations}
		};
    }
};